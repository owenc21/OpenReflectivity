#include <iostream>
#include <string>

#include "decoder.hpp"

int main(){
	std::string file_name = "archives/KEPZ20240423_041534_V06";
	archive_file file;
	int decode = Decoder::DecodeArchive(file_name, true, file);
	int x = file.header->version;
	// std::cout << x << std::endl;
}