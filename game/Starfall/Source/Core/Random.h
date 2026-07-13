#pragma once
#include <cstdint>
#include <random>

class Random
{
public:
    explicit Random(uint32_t seed) : engine(seed) {}

    int NextInt(int min, int max) { return std::uniform_int_distribution<int>(min, max)(engine); }

    float NextFloat(float min, float max) { return std::uniform_real_distribution<float>(min, max)(engine); }

private:
    std::mt19937 engine;
};
