#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <memory>
#include <zlib.h>
#include <bzlib.h>

#include "decoder.hpp"
#include "lvltwodef.hpp"


int Decoder::DecodeArchive(const std::string &file_name, archive_file &file){
	std::ifstream archive(file_name, std::ios::binary);
	if(!archive.is_open()){
		std::cerr << "Error opening the archive file." << std::endl;
		return -1;
	}

	// TODO: check for gzip compression (currently not supported )

	/* Parse the volume header */
	file.header = std::make_unique<volume_header>();

	char header_constant[7];
	archive.read(header_constant, 6);
	header_constant[6] = '\0';

	// Check for 'AR2V00' indicator
	if(std::string(header_constant) != "AR2V00"){
		std::cerr << "File is either corrupt or not a NEXRAD Level 2 archive file." << std::endl;
		return -1;
	}

	char version_raw[3];
	archive.read(version_raw, 2);
	version_raw[2] = '\0';
	uint8_t version = static_cast<uint8_t>(std::stoi(version_raw));

	archive.ignore(1);
	char ext_num_raw[4];
	archive.read(ext_num_raw, 3);
	ext_num_raw[3] = '\0';
	uint8_t ext_num = static_cast<uint8_t>(std::stoi(ext_num_raw));

	uint32_t date;
	archive.read(reinterpret_cast<char*>(&date), 4);
	date = reverseEndian(date);

	uint32_t time;
	archive.read(reinterpret_cast<char *>(&time), 4);
	time = reverseEndian(time);

	char icao[5];
	archive.read(icao, 4);
	icao[4] = '\0';

	file.header->version = version;
	file.header->extension_num = ext_num;
	file.header->date = date;
	file.header->time = time;
	file.header->icao = std::string(icao);

	return 0;
}

