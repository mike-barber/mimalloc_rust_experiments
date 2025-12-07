#include <cstdlib>
#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>
#include <iostream>

#include "mimalloc.h"
#include <random>

static std::atomic<long> gsum;

const int LEN[] = {1000, 5000, 10000, 50000};

// adapted from example in
// https://github.com/microsoft/mimalloc/issues/1104

static void local_alloc()
{
  // thread-local random number generator
  std::minstd_rand rng(std::random_device{}());

  long sum = 0;
  for (int i = 0; i < 500000; i++)
  {
    int len = LEN[rng() % 4];
    int *p = (int *)mi_zalloc_aligned(len * sizeof(int), alignof(int));
    p[0] = 1;
    sum += p[rng() % len];
    free(p);
  }
  std::cout << ".";
  gsum += sum;
}

static void test_thread_leak()
{
  std::vector<std::thread> threads;
  for (int i = 0; i < 24; ++i)
  {
    threads.emplace_back(std::thread(&local_alloc));
  }
  for (auto &th : threads)
  {
    th.join();
  }
  std::cout << "\n";
}

int main()
{
  auto start = std::chrono::high_resolution_clock::now();
  test_thread_leak();
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "took: " << duration.count() << " ms\n";
  std::cout << "gsum: " << gsum.load() << "\n";
  return 0;
}