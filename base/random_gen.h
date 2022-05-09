#include <random>

static unsigned int getRandomNumber() {
  std::random_device seed;
  return seed();
}