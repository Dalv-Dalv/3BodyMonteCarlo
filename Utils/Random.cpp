#include "Random.h"
Random* Random::instance;

Random::Random() : rng(dev()), dist(0.0f, 1.0f) {}

float Random::GetFloat(float min,float max) {
    CreateInstance();
    float x=Random::instance->dist(Random::instance->rng);
    return min+x*(max-min);
}

int64_t Random::GetInt(int64_t min, int64_t max) {
    CreateInstance();
    std::uniform_int_distribution<int64_t> dist(min, max);
    return dist(Random::instance->rng);
}

void Random::randomPointInUnitCircle(float& x, float& y) {
	float angle = 2.0f * M_PI * instance->GetFloat();
	float radius = std::sqrt(instance->GetFloat());

	x = radius * std::cos(angle);
	y = radius * std::sin(angle);
}

void Random::CreateInstance() {
    if (instance == nullptr) {
        Random::instance = new Random();
    }
}


