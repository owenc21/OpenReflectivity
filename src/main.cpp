#include <iostream>
#include <string>

#include "decoder.hpp"

int main(){
	std::string file_name = "archives/KDIX20240517_025206_V06";
	archive_file file;
	int decode = Decoder::DecodeArchive(file_name, file);
	int x = file.header->version;
	std::cout << x << std::endl;
}