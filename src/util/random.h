#pragma once
#include <iostream>
#include <random>
class Random {
public:
  Random(int min_num, int max_num_not_include, int seed)
      : gen(seed), distr(min_num, max_num_not_include - 1) {}
  inline int CreateRandom() { return distr(gen); }

private:
  std::mt19937_64 gen;
  std::uniform_int_distribution<int> distr;
};
