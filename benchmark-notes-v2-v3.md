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

# Run logs

## cpp mimalloc v2

```
mimalloc: process init: 0x72EDFC8DE740
mimalloc: using 1 numa regions
mimalloc: v2.2.4, release (built on Dec  6 2025, 22:05:44)
mimalloc: option 'show_errors': 0 
mimalloc: option 'show_stats': 1 
mimalloc: option 'verbose': 1 
mimalloc: option 'eager_commit': 1 
mimalloc: option 'arena_eager_commit': 2 
mimalloc: option 'purge_decommits': 1 
mimalloc: option 'allow_large_os_pages': 2 
mimalloc: option 'reserve_huge_os_pages': 0 
mimalloc: option 'reserve_huge_os_pages_at': -1 
mimalloc: option 'reserve_os_memory': 0 KiB
mimalloc: option 'deprecated_segment_cache': 0 
mimalloc: option 'deprecated_page_reset': 0 
mimalloc: option 'abandoned_page_purge': 0 
mimalloc: option 'deprecated_segment_reset': 0 
mimalloc: option 'eager_commit_delay': 1 
mimalloc: option 'purge_delay': 10 
mimalloc: option 'use_numa_nodes': 0 
mimalloc: option 'disallow_os_alloc': 0 
mimalloc: option 'os_tag': 100 
mimalloc: option 'max_errors': 32 
mimalloc: option 'max_warnings': 32 
mimalloc: option 'max_segment_reclaim': 10 
mimalloc: option 'destroy_on_exit': 0 
mimalloc: option 'arena_reserve': 1048576 KiB
mimalloc: option 'arena_purge_mult': 10 
mimalloc: option 'purge_extend_delay': 1 
mimalloc: option 'abandoned_reclaim_on_free': 0 
mimalloc: option 'disallow_arena_alloc': 0 
mimalloc: option 'retry_on_oom': 400 
mimalloc: option 'visit_abandoned': 0 
mimalloc: option 'guarded_min': 0 
mimalloc: option 'guarded_max': 1073741824 
mimalloc: option 'guarded_precise': 0 
mimalloc: option 'guarded_sample_rate': 0 
mimalloc: option 'guarded_sample_seed': 0 
mimalloc: option 'target_segments_per_thread': 0 
mimalloc: option 'generic_collect': 10000 
mimalloc: debug level : 0
mimalloc: secure level: 0
mimalloc: mem tracking: none
mimalloc: reserved 1048576 KiB memory
mimalloc: reserved 1048576 KiB memory
heap stats:     peak       total     current       block      total#   
  reserved:     2.0 GiB     2.0 GiB     2.0 GiB                          
 committed:     2.0 GiB     2.0 GiB     2.0 GiB                          
     reset:     0      
    purged:     0      
   touched:    75.8 MiB   101.1 MiB  -561.3 GiB                          
  segments:     1.1 Ki      1.5 Ki      1                                not all freed
-abandoned:     3          10           1                                not all freed
   -cached:     0           0           0                                ok
     pages:     0           0          -2.8 Mi                           not all freed
-abandoned:     3          10           1                                not all freed
 -extended:     0      
   -retire:     0      
    arenas:     2      
 -rollback:     0      
     mmaps:    28      
   commits:     0      
    resets:     0      
    purges:     0      
   guarded:     0      
   threads:    24          24           0                                ok
  searches:     0.0 avg
numa nodes:     1
   elapsed:     0.698 s
   process: user: 16.218 s, system: 0.051 s, faults: 0, rss: 23.8 MiB, commit: 2.0 GiB
mimalloc: process done: 0x72EDFC8DE740
........................
took: 697 ms
gsum: 3935
```

## cpp mimalloc v3

```
mimalloc: process init: 0x74B7A70D5780
mimalloc: warning: unable to allocate aligned OS memory directly, fall back to over-allocation (size: 0x20000 bytes, address: 0x74B7A70B5000, alignment: 0x10000, commit: 1)
mimalloc: v3.1.5, release (built on Dec  6 2025, 22:09:34)
mimalloc: option 'show_errors': 0 
mimalloc: option 'show_stats': 1 
mimalloc: option 'verbose': 1 
mimalloc: option 'eager_commit': 1 
mimalloc: option 'arena_eager_commit': 2 
mimalloc: option 'purge_decommits': 1 
mimalloc: option 'allow_large_os_pages': 2 
mimalloc: option 'reserve_huge_os_pages': 0 
mimalloc: option 'reserve_huge_os_pages_at': -1 
mimalloc: option 'reserve_os_memory': 0 KiB
mimalloc: option 'deprecated_segment_cache': 0 
mimalloc: option 'deprecated_page_reset': 0 
mimalloc: option 'abandoned_page_purge': 0 
mimalloc: option 'deprecated_segment_reset': 0 
mimalloc: option 'eager_commit_delay': 1 
mimalloc: option 'purge_delay': 1000 
mimalloc: option 'use_numa_nodes': 0 
mimalloc: option 'disallow_os_alloc': 0 
mimalloc: option 'os_tag': 100 
mimalloc: option 'max_errors': 32 
mimalloc: option 'max_warnings': 32 
mimalloc: option 'deprecated_max_segment_reclaim': 10 
mimalloc: option 'destroy_on_exit': 0 
mimalloc: option 'arena_reserve': 1048576 KiB
mimalloc: option 'arena_purge_mult': 1 
mimalloc: option 'deprecated_purge_extend_delay': 1 
mimalloc: option 'disallow_arena_alloc': 0 
mimalloc: option 'retry_on_oom': 400 
mimalloc: option 'visit_abandoned': 0 
mimalloc: option 'guarded_min': 0 
mimalloc: option 'guarded_max': 1073741824 
mimalloc: option 'guarded_precise': 0 
mimalloc: option 'guarded_sample_rate': 0 
mimalloc: option 'guarded_sample_seed': 0 
mimalloc: option 'generic_collect': 10000 
mimalloc: option 'page_reclaim_on_free': 0 
mimalloc: option 'page_full_retain': 2 
mimalloc: option 'page_max_candidates': 4 
mimalloc: option 'max_vabits': 0 
mimalloc: option 'pagemap_commit': 0 
mimalloc: option 'page_commit_on_demand': 0 
mimalloc: option 'page_max_reclaim': -1 
mimalloc: option 'page_cross_thread_max_reclaim': 32 
mimalloc: debug level : 0
mimalloc: secure level: 0
mimalloc: mem tracking: none
mimalloc: reserved 1048576 KiB memory
mimalloc: using 1 numa regions
heap stats:     peak       total     current       block      total#   
  reserved:     1.0 GiB     1.0 GiB     1.0 GiB                          
 committed:    58.3 MiB    58.5 MiB    32.5 MiB                          
     reset:     0      
    purged:    25.8 MiB
   touched:     0           0           0                                ok
     pages:     2.8 Mi      2.8 Mi      1                                not all freed
-abandoned:     1.1 Ki      2.8 Mi      1                                not all freed
 -reclaima:     4      
 -reclaimf:     0      
-reabandon:     0      
    -waits:     0      
 -extended:     0      
   -retire:     0      
    arenas:     1      
 -rollback:     0      
     mmaps:     6      
   commits:     0      
    resets:     0      
    purges:    95      
   guarded:     0      
   threads:    24          24           0                                ok
  searches:     1.0 avg
numa nodes:     1
   elapsed:     8.134 s
   process: user: 190.900 s, system: 0.236 s, faults: 0, rss: 21.5 MiB, commit: 58.3 MiB
mimalloc: process done: 0x74B7A70D5780
........................
took: 8134 ms
gsum: 3856
```

## rust mimalloc v2

```
    Finished `release` profile [optimized] target(s) in 0.33s
     Running `/home/michael/projects/mimalloc-rust-tests/target/release/rust-bench`
mimalloc: v2.2.4 (built on Dec  6 2025, 16:20:36)
mimalloc: option 'show_errors': 0 
mimalloc: option 'show_stats': 1 
mimalloc: option 'verbose': 1 
mimalloc: option 'eager_commit': 1 
mimalloc: option 'arena_eager_commit': 2 
mimalloc: option 'purge_decommits': 1 
mimalloc: option 'allow_large_os_pages': 2 
mimalloc: option 'reserve_huge_os_pages': 0 
mimalloc: option 'reserve_huge_os_pages_at': -1 
mimalloc: option 'reserve_os_memory': 0 KiB
mimalloc: option 'deprecated_segment_cache': 0 
mimalloc: option 'deprecated_page_reset': 0 
mimalloc: option 'abandoned_page_purge': 0 
mimalloc: option 'deprecated_segment_reset': 0 
mimalloc: option 'eager_commit_delay': 1 
mimalloc: option 'purge_delay': 10 
mimalloc: option 'use_numa_nodes': 0 
mimalloc: option 'disallow_os_alloc': 0 
mimalloc: option 'os_tag': 100 
mimalloc: option 'max_errors': 32 
mimalloc: option 'max_warnings': 32 
mimalloc: option 'max_segment_reclaim': 10 
mimalloc: option 'destroy_on_exit': 0 
mimalloc: option 'arena_reserve': 1048576 KiB
mimalloc: option 'arena_purge_mult': 10 
mimalloc: option 'purge_extend_delay': 1 
mimalloc: option 'abandoned_reclaim_on_free': 0 
mimalloc: option 'disallow_arena_alloc': 0 
mimalloc: option 'retry_on_oom': 400 
mimalloc: option 'visit_abandoned': 0 
mimalloc: option 'guarded_min': 0 
mimalloc: option 'guarded_max': 1073741824 
mimalloc: option 'guarded_precise': 0 
mimalloc: option 'guarded_sample_rate': 0 
mimalloc: option 'guarded_sample_seed': 0 
mimalloc: option 'target_segments_per_thread': 0 
mimalloc: option 'generic_collect': 10000 
mimalloc: debug level : 0
mimalloc: secure level: 0
mimalloc: mem tracking: none
mimalloc: process init: 0x75D82775A780
mimalloc: using 1 numa regions
mimalloc: reserved 1048576 KiB memory
........................
took: 698
gsum: 3960
heap stats:     peak       total     current       block      total#   
  reserved:     1.0 GiB     1.0 GiB     1.0 GiB                          
 committed:     1.0 GiB     1.0 GiB   224.1 MiB                          
     reset:     0      
    purged:   799.8 MiB
   touched:    69.6 MiB    96.4 MiB  -560.5 GiB                          
  segments:     1.1 Ki      1.5 Ki      1                                not all freed
-abandoned:     3          11           0                                ok
   -cached:     0           0           0                                ok
     pages:     0           0          -2.8 Mi                           not all freed
-abandoned:     4          13           0                                ok
 -extended:     0      
   -retire:     0      
    arenas:     1      
 -rollback:     0      
     mmaps:    25      
   commits:     0      
    resets:     0      
    purges:     3      
   guarded:     0      
   threads:    24          24           0                                ok
  searches:     0.0 avg
numa nodes:     1
   elapsed:     0.699 s
   process: user: 16.312 s, system: 0.182 s, faults: 0, rss: 52.6 MiB, commit: 1.0 GiB
mimalloc: process done: 0x75D82775A780
```

## rust mimalloc v3

```
mimalloc: v3.1.5 (built on Dec  6 2025, 16:21:16)
mimalloc: option 'show_errors': 0 
mimalloc: option 'show_stats': 1 
mimalloc: option 'verbose': 1 
mimalloc: option 'eager_commit': 1 
mimalloc: option 'arena_eager_commit': 2 
mimalloc: option 'purge_decommits': 1 
mimalloc: option 'allow_large_os_pages': 2 
mimalloc: option 'reserve_huge_os_pages': 0 
mimalloc: option 'reserve_huge_os_pages_at': -1 
mimalloc: option 'reserve_os_memory': 0 KiB
mimalloc: option 'deprecated_segment_cache': 0 
mimalloc: option 'deprecated_page_reset': 0 
mimalloc: option 'abandoned_page_purge': 0 
mimalloc: option 'deprecated_segment_reset': 0 
mimalloc: option 'eager_commit_delay': 1 
mimalloc: option 'purge_delay': 1000 
mimalloc: option 'use_numa_nodes': 0 
mimalloc: option 'disallow_os_alloc': 0 
mimalloc: option 'os_tag': 100 
mimalloc: option 'max_errors': 32 
mimalloc: option 'max_warnings': 32 
mimalloc: option 'deprecated_max_segment_reclaim': 10 
mimalloc: option 'destroy_on_exit': 0 
mimalloc: option 'arena_reserve': 1048576 KiB
mimalloc: option 'arena_purge_mult': 1 
mimalloc: option 'deprecated_purge_extend_delay': 1 
mimalloc: option 'disallow_arena_alloc': 0 
mimalloc: option 'retry_on_oom': 400 
mimalloc: option 'visit_abandoned': 0 
mimalloc: option 'guarded_min': 0 
mimalloc: option 'guarded_max': 1073741824 
mimalloc: option 'guarded_precise': 0 
mimalloc: option 'guarded_sample_rate': 0 
mimalloc: option 'guarded_sample_seed': 0 
mimalloc: option 'generic_collect': 10000 
mimalloc: option 'page_reclaim_on_free': 0 
mimalloc: option 'page_full_retain': 2 
mimalloc: option 'page_max_candidates': 4 
mimalloc: option 'max_vabits': 0 
mimalloc: option 'pagemap_commit': 0 
mimalloc: option 'page_commit_on_demand': 0 
mimalloc: option 'page_max_reclaim': -1 
mimalloc: option 'page_cross_thread_max_reclaim': 32 
mimalloc: debug level : 0
mimalloc: secure level: 0
mimalloc: mem tracking: none
mimalloc: process init: 0x79BBB2BAB7C0
mimalloc: reserved 1048576 KiB memory
mimalloc: using 1 numa regions
........................
took: 8212
gsum: 3906
heap stats:     peak       total     current       block      total#   
  reserved:     1.0 GiB     1.0 GiB     1.0 GiB                          
 committed:    64.6 MiB    64.6 MiB  -128.5 KiB                          
     reset:     0      
    purged:    64.7 MiB
   touched:     0           0           0                                ok
     pages:     2.8 Mi      2.8 Mi      2                                not all freed
-abandoned:     1.1 Ki      2.8 Mi      0                                ok
 -reclaima:     0      
 -reclaimf:     0      
-reabandon:     0      
    -waits:     0      
 -extended:     0      
   -retire:     0      
    arenas:     1      
 -rollback:     0      
     mmaps:     3      
   commits:     0      
    resets:     0      
    purges:   229      
   guarded:     0      
   threads:    24          24           0                                ok
  searches:     1.0 avg
numa nodes:     1
   elapsed:     8.217 s
   process: user: 193.125 s, system: 0.223 s, faults: 0, rss: 31.6 MiB, commit: 64.6 MiB
mimalloc: process done: 0x79BBB2BAB7C0
```
