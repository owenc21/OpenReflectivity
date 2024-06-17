#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "decoder.hpp"
#include "lvltwodef.hpp"

/* Utility Functions */
std::vector<uint8_t> readBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the file - '" + path + "'");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Error reading the file - '" + path + "'");
    }

    return buffer;
}
/* End Utility Functions */

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

// Tests successfully decompressing a Gzip-compressed archive file
TEST(GzipDecompress, DecompressesGzipFile){
	Decoder::ArchiveFile file("gz2archives/KDIX20240517_025206_V06.gz", true, false);
	std::vector<uint8_t> decompressed = file.getAll();
	std::vector<uint8_t> compare = readBinaryFile("archives/KDIX20240517_025206_V06");
	EXPECT_EQ(decompressed, compare);
}

// Tests successfully loading a non-Gzip-compressed archive file
TEST(GzipDecompress, NoDecompressGzipFile){
	Decoder::ArchiveFile file("archives/KDIX20240517_025206_V06", true, false);
	std::vector<uint8_t> decompressed = file.getAll();
	std::vector<uint8_t> compare = readBinaryFile("archives/KDIX20240517_025206_V06");
	EXPECT_EQ(decompressed, compare);
}

// Tests successfully decompressing one block of Bzip2 compression (a compressed archive file)
TEST(Bzip2Decompress, DecompressOneBlock){
	Decoder::ArchiveFile file("bzip2archives/KDIX20240517_025206_V06.prep", false, true);
	std::vector<uint8_t> decompressed = file.getAll();
	std::vector<uint8_t> compare = readBinaryFile("archives/KDIX20240517_025206_V06");
	EXPECT_EQ(decompressed, compare);
}

// Tests successfully parsing header of archive file
TEST(ParseFile, ParseFileHeader){
	archive_file file;
	std::string file_name = "archives/KDIX20240517_025206_V06";
	Decoder::DecodeArchive(file_name, file);
	EXPECT_EQ(6, file.header->version) << "Expected: 6 but Got: " << file.header->version;
	EXPECT_EQ(50, file.header->extension_num) << "Expected: 50 but Got: " << file.header->extension_num;
	EXPECT_EQ("KDIX", file.header->icao) << "Expected: \"KDIX\" but Got: \"" << file.header->icao << "\"";
}