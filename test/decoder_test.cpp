#include <gtest/gtest.h>
#include <iostream>

#include "decoder.hpp"

// Tests reversing endianness of one-byte data types
TEST(ReverseEndianTest, HandlesOneByte){
	uint8_t reverseOne = Decoder::reverseEndian<uint8_t>(0xAB);
	EXPECT_EQ(0xAB, reverseOne) << "Expected: " << std::hex << 0xAB << " but Got: " << reverseOne;
	uint8_t reverseTwo = Decoder::reverseEndian<uint8_t>(0x01);
	EXPECT_EQ(0x01, reverseTwo) << "Expected: " << std::hex << 0x01 << " but Got: " << reverseTwo;
	uint8_t reverseThree = Decoder::reverseEndian<uint8_t>(0x10);
	EXPECT_EQ(0x10, reverseThree) << "Expected: " << std::hex << 0x10 << " but Got: " << reverseThree;
	uint8_t reverseFour = Decoder::reverseEndian<uint8_t>(0x00);	
	EXPECT_EQ(0x00, reverseFour) << "Expected: " << std::hex << 0x00 << " but Got: " << reverseFour << std::dec;
}

// Tests reversing endianness of two-byte data types
TEST(ReverseEndianTest, HandlesTwoByte){
	uint16_t reverseOne = Decoder::reverseEndian<uint16_t>(0xABCD);
	EXPECT_EQ(0xCDAB, reverseOne) << "Expected: " << std::hex << 0xCDAB << " but Got: " << reverseOne;
	uint16_t reverseTwo = Decoder::reverseEndian<uint16_t>(0x0110);
	EXPECT_EQ(0x1001, reverseTwo) << "Expected: " << std::hex << 0x1001 << " but Got: " << reverseTwo;
	uint16_t reverseThree = Decoder::reverseEndian<uint16_t>(0x1001);
	EXPECT_EQ(0x0110, reverseThree) << "Expected: " << std::hex << 0x0110 << " but Got: " << reverseThree;
	uint16_t reverseFour = Decoder::reverseEndian<uint16_t>(0x0001);	
	EXPECT_EQ(0x0100, reverseFour) << "Expected: " << std::hex << 0x0100 << " but Got: " << reverseFour << std::dec;
	uint16_t reverseFive = Decoder::reverseEndian<uint16_t>(0x0000);	
	EXPECT_EQ(0x0000, reverseFive) << "Expected: " << std::hex << 0x0000 << " but Got: " << reverseFive << std::dec;
}

// Tests reversing endianness of four-byte data types
TEST(ReverseEndianTest, HandlesFourByte){
	uint32_t reverseOne = Decoder::reverseEndian<uint32_t>(0x1234ABCD);
	EXPECT_EQ(0xCDAB3412, reverseOne) << "Expected: " << std::hex << 0xCDAB3412 << " but Got: " << reverseOne;
	uint32_t reverseTwo = Decoder::reverseEndian<uint32_t>(0x01101001);
	EXPECT_EQ(0x01101001, reverseTwo) << "Expected: " << std::hex << 0x01101001 << " but Got: " << reverseTwo;
	uint32_t reverseThree = Decoder::reverseEndian<uint32_t>(0x100100FF);
	EXPECT_EQ(0xFF000110, reverseThree) << "Expected: " << std::hex << 0xFF000110 << " but Got: " << reverseThree;
	uint32_t reverseFour = Decoder::reverseEndian<uint32_t>(0x00010000);	
	EXPECT_EQ(0x00000100, reverseFour) << "Expected: " << std::hex << 0x00000100 << " but Got: " << reverseFour << std::dec;
	uint32_t reverseFive = Decoder::reverseEndian<uint32_t>(0x00000000);	
	EXPECT_EQ(0x00000000, reverseFive) << "Expected: " << std::hex << 0x00000000 << " but Got: " << reverseFive << std::dec;
}

// Tests reversing endianness of eight-byte data types
TEST(ReverseEndianTest, HandlesEightByte){
	uint64_t reverseOne = Decoder::reverseEndian<uint64_t>(0x123456789ABCDEFF);
	EXPECT_EQ(0xFFDEBC9A78563412, reverseOne) << "Expected: " << std::hex << 0xFFDEBC9A78563412 << " but Got: " << reverseOne;
	uint64_t reverseTwo = Decoder::reverseEndian<uint64_t>(0x0110100110100110);
	EXPECT_EQ(0x1001101001101001, reverseTwo) << "Expected: " << std::hex << 0x1001101001101001 << " but Got: " << reverseTwo;
	uint64_t reverseFive = Decoder::reverseEndian<uint64_t>(0x0000000000000000);	
	EXPECT_EQ(0x0000000000000000, reverseFive) << "Expected: " << std::hex << 0x0000000000000000 << " but Got: " << reverseFive << std::dec;
}