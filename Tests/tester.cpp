#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "APOverdrive.h"

TEST_CASE("DSP Soft Clipping Test")
{
  APOverdrive overdrive;
  auto sample = 1.0f;
  auto sampleOriginCopy = sample;
  overdrive.process(&sample, 1.0f, &sample, 1);
  REQUIRE(sampleOriginCopy != sample);
}