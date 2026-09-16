# Asynchronous servers

`runtime/program.cpp` owns startup/shutdown. `create_servers` checks SDL synchronization, thread and TLS initialization.
Each lane binds a stack-owned `ThreadContext` with its index/count and two scratch arenas. Gameplay stays on the main thread.

Content owns immutable bytes. Artifact Cache owns asynchronous computation, deduplication and publication. File Stream is
a built-in artifact producer, not a second scheduler. Content needs no maintenance tick in V1.

## Commands and ownership

```cpp
ArtifactCommandBuffer* commands = commands_begin(servers.artifacts.commands);
if (commands) {
    uint64_t request = file_load(*commands, path);
    commands_submit(*commands);
}

ArtifactEvent event;
while (command_next_event(servers.artifacts.commands, event)) {
    if (event.kind == ArtifactKind::File && event.result.state == ArtifactState::Ready) {
        ContentView bytes = content_get(servers.content, event.result.content);
        // Consume bytes on the main thread.
    }
}
```

Begin/record/submit/poll belong to one main thread. All producers share one stream and monotonically assigned request IDs.
Commands and copied paths occupy chained arenas; chunks hold 64 commands and separate result slots. Submission transfers
ownership. Do not touch a submitted buffer. Consuming its final event returns it to the pool and resets its arena.
Empty/unsubmitted buffers can be explicitly discarded. Completion order across buffers is unspecified.

| Boundary | Topology | Capacity/full behavior |
| --- | --- | --- |
| Recording pool | Main thread only | `command_buffers` buffers, default 32. Begin returns null if occupied, including unread results. |
| Recording buffer | Main thread only | `commands_per_buffer`, default 4096. Request returns 0 without appending when full. |
| Submission | Main broadcasts to one SPSC ring per lane | Each ring holds `command_buffers` pointers. Beginning a buffer reserves its queue slot. Submit cannot exhaust capacity. |
| Completion | Last finishing lane produces; main consumes (MPSC) | Same reservation guarantees space for every accepted buffer; workers never wait for event consumption. |

Backpressure means poll results/discard recording buffers and retry; never wait indefinitely for queue space on gameplay's behalf.
Variable-length path bytes are owned by the recording buffer, not retained from the caller. Queue memory never grows;
recording arenas grow only for explicit caller batches. IO/output allocations are coarse loading operations through SDL/Arena.

## MCBD execution and shutdown

Every lane executes the same artifact tick; each artifact identity hashes to one owning lane. Independent thin operations run
across lanes; identical identities serialize at that owner and reuse its published result. Lane index/count come from TLS.
Queue locks cover pointer transfer, lane read/write locks cover index/state publication, and content has 16 striped indexes.
Neither filesystem calls nor computation hold an artifact index lock. Workers only write cache state and event slots.

V1 has **thin producers only**. SDL IO streams lack positional reads; whole-file reads each own an independent handle.
There is no speculative wide-work API or producer barrier. A future wide producer must explicitly arrange matching participation
before introducing internal barriers; it cannot be dropped into a thin owner-only dispatch.

The **one group rendezvous** is after lanes drain work, before idle/stop. It establishes that no producer is still executing
before the group sleeps or exits. The last arriving lane waits on SDL's condition variable; other lanes wait for the group phase
to advance. Submission advances a generation under the same mutex, preventing lost wakeups. There are no barriers between ticks.

`stop_servers` rejects new submission, wakes sleepers, drains accepted work and joins all lanes. It preserves events/content
for polling afterward and never waits for main-thread event consumption. `destroy_servers` additionally discards outstanding
recordings/results and destroys caches. Outstanding filesystem calls must return; there is no forced cancellation.

## Immutable content and artifacts

`ContentHash` is SHA-256 of exact bytes. Concurrent identical insertions retain one blob. Sorted striped indexes grow in coarse
blocks; blob pointers stay stable. `content_publish` transfers an entire arena without copying; deduplication destroys the incoming
arena. `content_insert` copies caller bytes only on a miss. All content and ready artifacts remain resident until cache shutdown.

Persist hashes, not raw data pointers. `ContentView` is borrowed for the current cache-use phase, not an owning handle. Cache maintenance
requires quiescent readers; the only V1 maintenance is destruction after readers stop. This leaves an explicit owner-controlled boundary
for later eviction without changing persistent identities. There are no reference counts, history, LRU, disk cache or synchronous gameplay waits.

Derived keys hash source content, parameter content, artifact kind and implementation version with canonical scalar encoding.
`artifact_request` schedules typed data. `artifact_get` observes Missing/Building/Ready/Failed without waiting for computation.
Duplicate requests queued while a build runs share its result; no second successful computation occurs. Failures can be retried.
Changed inputs produce a **different key**: keep the previous ready key until the replacement becomes ready. Starting or failing
the replacement never destroys the old artifact. No logical alias can change the meaning of an immutable computation key.

`ByteHistogram` v1 produces immutable bytes; `ImportGLTF` v2 produces an arena-owned ImportedScene and one CPU MeshAsset per imported mesh.
The [asset factory](assets.md) retains current/pending generations and publishes on the main thread. Producers extend typed dispatch, not public callbacks.

## File identity and validation

`file_identity(path)` is a domain-separated path digest, not a content hash lookup. Exact path spelling defines identity;
use stable absolute paths. Different paths can publish the same immutable content. Load commands probe properties again, allowing
explicit refresh without a watcher. Observed size/type/modification-time changes increment the file generation.

An unchanged successful file reuses its content. A changed file retains `content` and `ready_generation` while `generation` advances
and state becomes Building; failure also leaves the previous ready content usable. The event identifies the request, file identity,
generation, state/error and content. Inspect `ready_generation` to distinguish usable stale data from a failed first load.

Reads validate size/type/modification time before and after IO and publish only validated bytes. Detected changes discard the arena
and retry at most three reads, then report Changed. The default configurable size limit is 1 GiB. Missing files, invalid file types,
IO errors and excessive size produce explicit failures. This is best-effort validation, not an atomic filesystem snapshot: changes
that preserve all observed properties can escape detection. Whole-file loading only; no ranges, watching or generic retry framework.

## RAD reference and deliberate differences

Inspected RAD revision `c762d1cd7106db2b97f7edfa5e0a2bef39c3a12b`:
[content](https://github.com/EpicGames/raddebugger/blob/c762d1cd7106db2b97f7edfa5e0a2bef39c3a12b/src/content/content.c),
[artifact_cache](https://github.com/EpicGames/raddebugger/blob/c762d1cd7106db2b97f7edfa5e0a2bef39c3a12b/src/artifact_cache/artifact_cache.c),
[file_stream](https://github.com/EpicGames/raddebugger/blob/c762d1cd7106db2b97f7edfa5e0a2bef39c3a12b/src/file_stream/file_stream.c),
plus the base ring/thread entry implementations.

Orbit keeps arena transfer, immutable publication, file-as-artifact production, duplicate suppression, pre/post validation and stale
results. It uses SHA-256 rather than RAD's 128-bit identity, resident caches rather than history/refcounts/eviction, deterministic thin
owners rather than wide/thin priority batches, and typed bounded command/result buffers rather than callable requests and deadline waits.
SDL whole-file IO does not justify RAD's cooperative positional reads. These choices leave wide producers, watching,
priorities, cancellation and eviction for concrete engine requirements.
