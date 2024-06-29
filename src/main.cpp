#include <iostream>
#include <string>

#include "decoder.hpp"

int main(){
	std::string file_name = "archives/KDIX20240623_234538_V06";
	archive_file file;
	int decode = Decoder::DecodeArchive(file_name, true, file);
	int x = file.header->version;
	// std::cout << x << std::endl;
}