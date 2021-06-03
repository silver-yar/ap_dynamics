#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "APOverdrive.h"

TEST_CASE("DSP Overdrive Test")
{
  APOverdrive overdrive;
  auto sampleArray = std::vector<float> {0.0f, 0.2f, 0.4f, 0.6f, 0.8f, 1.0f};
  std::vector<float> resultArray(sampleArray.size());
  float mix = 0.0f;

  SECTION("No Clipping") {

  }

  SECTION("Soft Clipping") {
    mix = 0.4f;
  }

  SECTION("Hard Clipping") {
    mix = 0.8f;
  }
}