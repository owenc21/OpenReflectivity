#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>

#include "decoder.hpp"
#include "zstr.hpp"

std::vector<uint8_t> readBinaryData(std::ifstream& stream, std::size_t numBytes) {
    std::vector<uint8_t> buffer(numBytes);
    stream.read(reinterpret_cast<char*>(buffer.data()), numBytes);
    return buffer;
}

int Decoder::DecodeFile(const std::string& file_name){
	std::ifstream archive(file_name, std::ios::binary);

	// unsigned char byte1;
	// unsigned char byte2;

	// byte1 = Decoder::readBinary<unsigned char>(archive);
	// byte2 = Decoder::readBinary<unsigned char>(archive);

	// std::cout << std::hex << byte1 << std::dec << ":" << std::hex << byte2 << std::dec << std::endl;

	// std::vector<uint8_t> firsteight = readBinaryData(archive, 8);
	// std::cout << std::hex << eightbytes << std::dec << std::endl;
	
    // std::cout << std::hex << std::uppercase;
    // for (unsigned char byte : firsteight) {
    //     std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    // }
    // std::cout << std::dec << std::endl;

	// if(eightbytes == 0x3630303056325241) std::cout << "0x3630303056325241" << std::endl;
	// else if(eightbytes == 0x4152325630303036) std::cout << "0x4152325630303036" << std::endl;

	// Decoder::myword theword;
	// theword = Decoder::readBinary<myword>(archive);

	// std::cout << std::hex << std::uppercase;
	// for(int i=0; i<8; i++){
	// 	std::cout << std::setw(2) << std::setfill('0') << static_cast<int>((theword.data >> (8*i)) & 0xFF);
	// }
	// std::cout << std::endl << theword.data;
	// uint64_t endswap = (theword.data >> 56) & 0xFF | (theword.data >> 40) & 0xFF00 | (theword.data >> 24) & 0xFF0000 | (theword.data >> 8) & 0xFF000000 | (theword.data << 8) & 0xFF00000000 | (theword.data >> 24) & 0xFF0000000000 | (theword.data << 40) & 0xFF000000000000 | (theword.data << 56) & 0xFF00000000000000;
	// std::cout << std::endl << endswap << std::dec << std::endl;

	uint64_t eight = Decoder::readBinary<uint64_t>(archive);

	std::cout << std::hex << std::uppercase;
	for(int i=0; i<8; i++){
		std::cout << std::setw(2) << std::setfill('0') << static_cast<int>((eight >> (8*i)) & 0xFF) << " ";
	}

	std::cout << std::endl;
	eight = Decoder::reverseEndian<uint64_t>(eight);

	for(int i=0; i<8; i++){
		std::cout << std::setw(2) << std::setfill('0') << static_cast<int>((eight >> (8*i)) & 0xFF) << " ";
	}

	std::cout << std::endl << std::dec;

	return 0;
}

