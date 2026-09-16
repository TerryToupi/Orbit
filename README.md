# Orbit
Yet another Game Engine!

The runtime entry point is `runtime/program.cpp`. See [asynchronous servers](docs/servers.md) for file streaming, immutable content,
artifact computation, command ownership and shutdown.
The [CPU static-mesh asset pipeline](docs/assets.md) adds asynchronous GLB import, stable mesh handles and explicit hot reload without GPU resources.
The [rendering compositor](docs/rendering.md) consumes an abstract Scene and owns SDL GPU uploads, depth/GBuffer passes, resizing, and presentation.

## Runtime memory

`runtime/arena.h` provides caller-owned chained arenas and `ArenaAllocator<T>`. Blocks use SDL's installed allocator;
growth preserves pointers. Reset retains the oldest block; destruction releases all blocks. Neither runs object destructors.

```cpp
Arena arena = {.block_capacity = 64 * 1024};
{
    std::vector<Foo, ArenaAllocator<Foo>> values{ArenaAllocator<Foo>{arena}};
    values.reserve(100);
}
destroy_arena(arena);
```

`runtime/thread_context.h` binds a caller-owned `ThreadContext` through SDL TLS. After SDL initialization, call
`set_thread_context(&context)` and check its result. Binding prepares two scratch arenas; configure their capacities beforehand.
The application binds its main context at startup. Each additional engine thread must bind and destroy its own context.

```cpp
ArenaTemp scratch = scratch_begin({&output_arena});
Foo* temporary = arena_allocate<Foo>(*scratch.arena, count); // uninitialized storage
scratch_end(scratch);
```

Use `scratch_begin()` without conflicts, or pass `{conflicts, count}` for an array. At least one scratch arena must be available.
End scopes in reverse order per arena and destroy containers before reclaiming their storage. Call `destroy_thread_context`
on its owner thread before exit or SDL shutdown; it clears the current binding and frees scratch blocks.

Configure with `-DORBIT_BUILD_TESTS=ON`, build `runtime_tests`, and run `ctest --test-dir <build-directory>` to test the foundation.
