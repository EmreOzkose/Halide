# WebAssembly Support for Halide

This branch represents an effort to support WebAssembly (wasm) code generation from Halide, using the Halide backend.

It is very much a work-in-progress at this point; be sure to read all of the current limitations. Some of the most important:

- SIMD isn't enabled by default (yet), mainly because existing supports in LLVM and/or V8 isn't quite solid enough yet.
- Multithreading is not yet ready to be supported, but we hope it will be soon.
- Halide's JIT for wasm is useful only for internal testing purposes.

# Additional Tooling Requirements:
- In additional to the usual install of LLVM and Clang, you'll need wasm-ld (via LLVM/tools/lld). All should be v9.x+ (current trunk)
- V8 library, v7.5+
- d8 shell tool, v7.5+
- Emscripten, 1.38.28+

Note that for all of the above, earlier versions might work, but have not been tested. (We intend to provide more concrete baseline requirements as development continues.)

# AOT Limitations

Halide outputs a wasm object (.o) or static library (.a) file, must like any other architecture; to use it, of course, you must link it to suitable calling code. Additionally, you must link to something that provides an implementation of `libc`; as a practical matter, this means using the Emscripten tool to do your linking, as it provides the most complete such implementation we're aware of.

- Halide ahead-of-time tests assume/require that you have Emscripten installed and available on your system.

- Halide doesn't support multithreading in wasm at this point; we obviously would like to do so, but there are practical issues with doing so at the time of this writing:

  - At present, support for Threads in wasm effectively requires compiling Emscripten with pthreads enabled and running in a complete browser environment; there are no command-line tools (e.g. Node, d8) that we're aware of which support threading in wasm. The Halide test build and test facility doesn't have any existing way to do in-browser testing, so we are focusing on single-threaded output until we have a way to reliably test.


# JIT Limitations

It's important to reiterate that the JIT mode will never be appropriate for anything other than limited self tests, for a number of reasons:

- It requires linking both an instance of the V8 library and LLVM's wasm-ld tool into libHalide. (We would like to offer support for other wasm engines in the future, e.g. SpiderMonkey, to provide more balanced testing, but there is no timetable for this.)
- Every JIT invocation requires redundant recompilation of the Halide runtime. (We hope to improve this once the LLVM wasm backend can support dlopen() properly.)
- wasm effectively runs in a private, 32-bit memory address space; while the host has access to that entire space, the reverse is not true, and thus any `define_extern` calls require copying all `halide_buffer_t` data across the wasm<->host boundary in both directions.
- Host functions used via `define_extern` or `HalideExtern` cannot accept or return values that are pointer types or 64-bit integer types; this includes things like `const char *` and `user_context`. (`halide_buffer_t*` is explicitly supported as a special case, however.)
- Threading isn't supported at all (yet); all `parallel()` schedules will be run serially.
- The `.async()` directive isn't supported at all, not even in serial-emulation mode.
- You can't use `Param<void *>` (or any other arbitrary pointer type) with the wasm jit.
- You can't use `Func.debug_to_file()`, `Func.set_custom_do_par_for()`, `Func.set_custom_do_task()`, or `Func.set_custom_allocator()`.
- The implementation of `malloc()` used by the JIT is incredibly basic and unsuitable for anything other than the most basic of tests.
- GPU usage isn't supported at all yet.

Note that while some of these limitations may be improved in the future, some are effectively intrinsic to the nature of this problem. Realistically, this JIT implementation is intended solely for running Halide self-tests (and even then, a number of them are fundamentally impractical to support in a hosted-wasm environment and are blacklisted).

In sum: don't plan on using Halide JIT mode with wasm unless you are working on the Halide library itself.
