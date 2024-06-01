#include <iostream>
#include <string>

#include "decoder.hpp"

int main(){
	std::string file_name = "generic file name";
	int decode = Decoder::DecodeFile(file_name);
	std::cout << decode << std::endl;
}