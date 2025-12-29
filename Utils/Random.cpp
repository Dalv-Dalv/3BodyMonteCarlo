#include "Random.h"
Random* Random::instance;

Random::Random() : rng(dev()), dist(0.0f, 1.0f) {}

float Random::GetFloat(float min,float max) {
    CreateInstance();
    float x=Random::instance->dist(Random::instance->rng);
    return (x-min)*(max-min);
}

int64_t Random::GetInt(int64_t min, int64_t max) {
    CreateInstance();
    std::uniform_int_distribution<int64_t> dist(min, max);
    return dist(Random::instance->rng);
}


void Random::CreateInstance() {
    if (instance == nullptr) {
        Random::instance = new Random();
    }
}


