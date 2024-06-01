#include <gtest/gtest.h>

#include "decoder.hpp"

TEST(DecoderTest, BasicTest){
	EXPECT_EQ(0, Decoder::DecodeFile("test"));
}