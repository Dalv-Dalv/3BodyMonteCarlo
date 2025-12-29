#pragma once
#include <random>

class Random {
public:
    static Random* instance;

    Random();
    Random(const Random&) = delete;
    Random& operator=(const Random&) = delete;

    static float GetFloat(float min=0,float max=1);
    static int64_t GetInt(int64_t min,int64_t max);

private:
    std::random_device dev;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;

    static void CreateInstance();
};
