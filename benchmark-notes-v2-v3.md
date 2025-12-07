# Setup

- AMD Ryzen 3900X running Linux under WSL
- Broadly similar results on M1 Macbook under OSX
- Broadly similar results on C++ side with gcc and clang

I modified the benchmarks a bit to better reflect a use case where I noticed the performance regression in the first place. I also used a fast thread-local random number generator in C++, as the shared rand() call used in the example above was responsible for the difference between Rust and C++ results. Now they're very close.

Note that I'm also using `mi_zalloc_aligned` in C++ because this is the function called from Rust when allocating a large zeroed `vec`. 

# C++ (cpp-bench)

- mimalloc v2 took: 708 ms
- mimalloc v3 took: 8201 ms

# Rust (rust-bench)

- mimalloc v2 took: 731 ms
- mimalloc v3 took: 8242 ms

# Profiling

Using `perf` on the Rust build reveals ~50% of cycles spent in `memset`. Changing the call in C++ from `mi_zalloc_aligned` to 
`mi_malloc_aligned` reduces the runtime significantly in the `mimalloc v3` case. 

With V3: 
- mi_zalloc_aligned: 8201 ms (as above - matches Rust use)
- mi_malloc_aligned: 427 ms

Although, of course, the results are then incorrect as we're expecting a zeroed allocation. But it does hint that the problem is _related_ to zeroing out the allocated chunk in v3. We're doing the same thing in v2, but it's a lot faster.

Note that the same results as `mi_zalloc_aligned` are obtained if we use `mi_malloc_aligned` and then zero the allocated memory using `memset`. Similar results if I zero the array using AVX2 writes, so it's not as if the zeroing itself is slow either.

Given the above, it seems like this might be a cache coherence thing. If new allocations aren't as local as they were in `v2` and often come from memory previously used by a different core, then perhaps what we're seeing here is the cost of that.