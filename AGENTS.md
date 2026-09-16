## Coding Guidelines

- Write simple, efficient, minimal C/C++ code.
- Do not add obvious comments such as "arguments must be live objects". C/C++ programmers already understand that using destroyed objects is invalid.
- Do not add asserts or comments describing 32-bit overflow cases. The existing 32-bit ranges (about 4 billion elements or 4 GB) are sufficient.
- Avoid adding named variables for trivial expressions, especially when the value is used only once.
- Avoid `auto` for local variables. Do not use it for ordinary value or structure types.
- Disable C++ exceptions and RTTI across the entire codebase. Do not use either.
- Perform error checks as early as possible. Check application initialization and resource loading creation immediately. Avoid error checking after initialization; normal code should not fail.
- Treat the library as a low-level, thin wrapper, not as a validation layer. Validate only inputs and state whose misuse could make the wrapper itself crash.
  Document those preconditions and enforce programmer errors with asserts.
- Data and parameters passed directly from the user to Vulkan are the user's responsibility. Do not duplicate Vulkan validation or maintain shadow state
  solely to validate them; users should enable the Vulkan validation layer during development.
- Orbit resource destruction is immediate. In applications, examples, and tests, destroy resources only after no recorded or executing GPU frame
  uses them. Wait for the submission timeline value covering the final use, or use the optional OrbitUtility `DeleteQueue` to defer destruction.
- At shutdown, call `wait_idle`, drain every OrbitUtility `DeleteQueue`, and then destroy resources and the device.
- Wait for every submitted frame to drain before calling `destroy_device`.
- Group APIs logically in public headers; for example, keep all command buffer APIs together.
- Keep each public resource creation function next to its matching destruction function. Keep the shared lifetime policy in one place rather than repeating it
  for individual resource types.
- Do not abort for programming errors. Enforce their documented preconditions with asserts and leave release builds free of those checks.
- Use error codes and error messages only for invalid external input data and initialization failures.
- Do not check for memory allocation failures. We cannot recover from running out of memory; managing memory usage properly is the user's responsibility.
- Avoid lambdas and complex templates.
- Avoid trivial single-line wrapper functions.
- Avoid trivial single-element wrapper structs.
- Avoid memory allocations, including short-lived local vectors.
- Do not use `std::shared_ptr`.
- Avoid `std::unique_ptr`, especially when the value can be stored directly as a data member.
- Do not use class inheritance or virtual functions.
- Do not use PIMPL interfaces.
- Avoid standard-library algorithms; prefer straightforward loops.
- Do not use hash maps or ordered maps.
- Avoid copying large user data structures. Prefer references to structures, and use spans for array data in structures and function parameters.
- Use a custom span type represented by a pointer and size. It must support construction from an initializer list so variable-length arguments remain concise. An initializer list passed as a function argument remains alive through that function call; do not retain a span backed by it after the call returns.
- Use C++20 designated initializers with named fields for structures.
- Give public API structure fields useful default values. At call sites, initialize only the non-default fields and name every initialized field.
- Always review code for performance issues before considering work complete.
- Line length is 160 characters. Please don't chop expressions to multiple lines if not needed.

## Data-Oriented Coding Guidelines

- Prefer plain structures and free functions over classes with behavior. Keep data representation explicit and easy to inspect.
- Prefer POD-like types. Avoid constructors, destructors, copy operators, and other implicit lifetime behavior unless they provide substantial value.
- Prefer aggregate initialization and C++20 designated initializers over constructors and builder patterns.
- Keep ownership explicit. A pointer or span does not imply ownership unless the API explicitly documents otherwise.
- Avoid hidden work. Copying, allocation, locking, reference counting, and resource destruction should be visible at the call site or obvious from the API.
- Avoid RAII when it hides significant resource lifetime or synchronization behavior. Explicit creation and destruction are preferred for engine resources.
- Avoid clever C++ abstractions. Prefer code whose generated work can be understood directly from the source.
- Do not introduce abstractions solely to reduce repetition. Small amounts of duplication are preferable to indirection in low-level code.
- Avoid deep call chains. Keep hot-path execution straightforward and easy to follow.
- Prefer concrete types and functions over generic programming. Use templates only where they remove meaningful duplication without obscuring generated code.
- Avoid type erasure, `std::function`, delegates that allocate, and other runtime-polymorphic abstractions.
- Avoid implicit conversions in low-level APIs when they make data movement or representation unclear.
- Prefer explicit integer widths for stored engine data and serialized/GPU-facing structures.
- Use `size_t` for byte sizes, allocation sizes, and pointer-relative memory calculations. Use fixed-width integers for intentionally bounded indices, counts, handles, and file-format fields.
- Keep structures compact. Consider field ordering, padding, alignment, cache-line usage, and access frequency when defining frequently instantiated structures.
- Separate frequently accessed data from rarely accessed data when doing so reduces cache traffic.
- Prefer structure-of-arrays or split hot/cold data when code processes large numbers of objects by individual fields.
- Do not store redundant derived state when it is cheap to recompute and would increase memory traffic or synchronization requirements.

## Memory Guidelines

- Treat heap allocation as an initialization, loading, or coarse-grained operation. Avoid general-purpose heap allocation in frame, tick, render, physics, and other hot paths.
- Prefer arenas for groups of allocations that share a lifetime.
- Prefer scratch arenas for temporary working memory. Reset or rewind them in bulk instead of freeing individual allocations.
- Prefer persistent arenas for long-lived subsystem state when individual deallocation is unnecessary.
- Arena allocation should be linear and cheap. Do not add per-allocation bookkeeping unless required by the subsystem.
- Arena growth should occur in coarse blocks. Do not resize or copy the existing arena contents when capacity is exceeded.
- Do not rely on arena allocations remaining contiguous across chained blocks.
- Temporary allocations must have an obvious lifetime boundary.
- Avoid allocating memory merely to transform data between internal representations. Prefer processing the existing representation directly when practical.
- Avoid allocation-owning standard-library containers in hot paths.
- Prefer caller-provided memory, arenas, fixed-capacity storage, or explicitly persistent storage over internally allocated temporary buffers.
- Reuse buffers and working memory across ticks when their lifetime naturally belongs to a subsystem.
- Avoid per-object heap allocation. Allocate objects in batches, pools, arenas, or contiguous arrays where practical.
- Prefer contiguous storage over pointer-linked structures.
- Avoid linked lists and pointer-heavy trees unless their behavior specifically requires them.
- Avoid storing pointers when an index or compact handle provides better locality and lifetime semantics.
- Do not zero-initialize large allocations unless the zeroed contents are actually required.
- Do not initialize memory that will immediately be completely overwritten.
- Avoid copying memory during container growth where a segmented, chained, reserved, or fixed-capacity representation is suitable.
- Large persistent allocations should have predictable ownership and lifetime. Avoid many independently allocated persistent objects.

## Hot-Path Guidelines

- Frame, tick, render submission, command recording, job execution, and similar frequently executed paths should perform no unexpected allocation.
- Keep hot loops simple and branch-light.
- Hoist invariant work out of loops.
- Cache repeated pointer dereferences and expensive calculations only when doing so meaningfully reduces work; do not introduce locals merely for stylistic reasons.
- Prefer linear iteration through contiguous memory.
- Avoid pointer chasing in performance-sensitive loops.
- Avoid callbacks, indirect calls, and function pointers inside tight loops unless the indirection is fundamental to the design.
- Avoid synchronization in inner loops.
- Batch work before crossing synchronization, thread, operating-system, or graphics-driver boundaries.
- Prefer coarse synchronization points over frequent fine-grained synchronization.
- Do not introduce atomics where ownership or work partitioning can remove the need for them.
- Prefer single-writer ownership of mutable data.
- Keep thread-local temporary state in the thread context rather than repeatedly allocating or synchronizing shared state.
- False sharing must be considered for frequently modified per-thread data. Separate independently written hot data when necessary.
- Do not optimize cold initialization code at the expense of runtime data layout or hot-path simplicity.

## Container Guidelines

- Prefer simple contiguous arrays and custom lightweight containers whose allocation behavior is explicit.
- A container used in performance-sensitive code should make its capacity, count, storage, and growth behavior obvious.
- Prefer fixed-capacity arrays when a meaningful upper bound exists.
- Reserve storage up front when the required or expected capacity is known.
- Avoid incremental geometric growth in hot paths when capacity can be determined earlier.
- Do not shrink container capacity automatically.
- Container destruction should not perform surprising recursive or individually allocated cleanup.
- Prefer indices into stable arrays over owning pointer graphs.
- Keep handles compact. Prefer integer handles containing an index and generation where stale-handle detection is required.
- Do not use containers whose iterator/reference stability requirements force unnecessary indirection.

## String Guidelines

- Avoid owning strings in hot-path and frequently instantiated structures.
- Prefer string views, interned strings, identifiers, or arena-backed strings according to lifetime requirements.
- Do not allocate for string formatting in hot paths.
- Prefer explicit string length over repeated null-terminated string scans when strings are processed frequently.
- Avoid repeated string comparison for runtime dispatch. Convert names to identifiers during loading or initialization where appropriate.
- Keep path normalization and other string processing at subsystem boundaries rather than repeating it during runtime lookup.

## Function and API Style

- Pass small scalar types by value and larger structures by reference.
- Prefer pointers when null is meaningful and references when it is not.
- Prefer output parameters or returned small structures over heap-allocated results.
- Avoid functions that return allocation-owning containers from low-level code.
- Make expensive operations recognizable from their names and APIs.
- Keep hot-path functions small enough that control flow and memory access are easy to understand, but do not split straightforward code into trivial wrapper functions.
- Prefer explicit context parameters over hidden global state when a subsystem naturally operates through a context.
- Do not create getter/setter APIs for plain internal data solely for encapsulation.
- Avoid fluent APIs and builder objects.
- Avoid overload sets that make materially different operations look identical.
- Avoid default arguments when they hide meaningful runtime work or resource behavior.
- Keep APIs orthogonal. Prefer a small number of composable primitive operations over many convenience variants.

## C++ Usage Restrictions

- Treat C++ primarily as a stronger type system and compile-time convenience layer over a C-style, data-oriented implementation.
- Do not use exceptions, RTTI, inheritance, virtual dispatch, coroutines, ranges, concepts-heavy interfaces, or metaprogramming-heavy abstractions.
- Avoid `std::function`, `std::any`, `std::variant`, and `std::optional` in low-level runtime code unless there is a strong representation-level reason.
- Avoid iostreams.
- Avoid standard-library synchronization primitives when the platform/runtime layer already provides the required primitive.
- Avoid standard-library containers when their allocation behavior or representation conflicts with subsystem memory requirements.
- Do not use smart pointers as a substitute for designing ownership.
- Avoid implicit dynamic initialization of global objects.
- Prefer compile-time constants and plain static data over runtime-initialized global objects.
- Keep translation-unit initialization trivial.
- Avoid operator overloading except for small mathematical types where the operation has obvious value semantics.

## Performance Review

- Before considering runtime code complete, inspect its allocation behavior, memory access pattern, synchronization, data copies, and number of indirect calls.
- For frequently executed code, reason about bytes moved and cache behavior in addition to instruction count.
- Question every allocation inside a loop.
- Question every lock, atomic operation, indirect call, and system call inside a hot path.
- Question every large structure copied by value.
- Question every pointer-heavy representation when the same data could be stored contiguously.
- Prefer removing work over making unnecessary work faster.
- Do not add caching without considering the memory cost, invalidation cost, and additional cache pressure.
- Performance-oriented complexity should be justified by measured or structurally obvious benefit. Keep the simpler implementation when the difference is insignificant.

## Testing Guidelines

* Keep all tests outside the `runtime/` directory. The runtime tree contains only source and data required to build and run the engine.
* Place tests in a top-level `tests/` directory that mirrors the relevant runtime structure where useful.
* Do not add `tests/`, `test/`, `benchmarks/`, `mocks/`, `fixtures/`, or similar directories inside `runtime/`.
* Do not place test-only source files, headers, assets, utilities, or dependencies in `runtime/`.
* Runtime code must never depend on test code or test-only libraries.
* Test infrastructure may depend on runtime code; the dependency must never go in the opposite direction.
* Keep test helpers local to `tests/` unless the functionality is genuinely required by the runtime.
* Do not modify a public runtime API solely to make it easier to test. Test through the real API where practical.
* Prefer small focused tests over large testing frameworks or heavily abstracted fixtures.
* Avoid allocation-heavy or inheritance-heavy testing frameworks and test abstractions where simple test functions are sufficient.
* Tests should exercise the same allocation, threading, and lifetime behavior used by real runtime code rather than replacing core behavior with mocks.
* Keep test assets under `tests/` or a dedicated top-level test-data directory. Do not mix them with runtime assets.
* Put performance benchmarks in a top-level `benchmarks/` directory, separate from correctness tests and runtime code.
* Benchmark release or optimized builds. Do not draw performance conclusions from debug builds.
* Performance-sensitive changes should be checked for allocation count, memory usage, data movement, and execution time where relevant.
* Tests and benchmarks must be optional build targets. Building the runtime must not require building or discovering test infrastructure.
* Keep test and benchmark dependencies out of the runtime target's include paths, compile definitions, and link dependencies.

## Runtime Directory Policy

* The runtime entry point is `runtime/program.cpp`, which defines the SDL3 application callbacks and owns application startup and shutdown.
* Treat `runtime/` as the shipping engine runtime, not as the repository root for engine-related development files.
* Every file and directory under `runtime/` must have a runtime purpose.
* Do not create organizational directories under `runtime/` unless they correspond to an actual runtime subsystem.
* Keep development tooling, tests, benchmarks, experiments, generated test data, documentation, and repository infrastructure outside `runtime/`.
* Do not create empty or speculative subsystem directories. Add directories only when runtime code requiring that separation exists.
* Keep the runtime directory hierarchy shallow. Prefer a small number of meaningful subsystem directories over deeply nested categorization.
* Do not mirror architectural concepts into directories when a few source files are sufficient.
* Examples and sample applications belong outside `runtime/` and consume the runtime through its public interface.
* Repository tooling and scripts belong in top-level development directories such as `tools/`, not under `runtime/`.

# Documentation Guidelines

- Keep Markdown documentation short, precise, and focused on user-facing behavior and fidelity to the *No Graphics API* design. Emphasize GPU pointers,
  the root ABI, and relevant Vulkan extensions. Avoid obvious C/C++ conventions, internal plumbing, exhaustive `Desc` or API catalogs, and sample-specific
  asset or format details better left in source files. Keep example descriptions brief.
