#include <iostream>
#include <string>

#include "decoder.hpp"

int main(){
	std::string file_name = "archives/KDIX20240517_025206_V06";
	int decode = Decoder::DecodeFile(file_name);
}