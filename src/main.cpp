#include <iostream>
#include <string>

#include "decoder.hpp"

int main(){
	std::string file_name = "gz2archives/KDIX20240517_025206_V06.gz";
	archive_file file;
	int decode = Decoder::DecodeArchive(file_name, file);
	int x = file.header->version;
	std::cout << x << std::endl;
}