#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <memory>
#include <cstdlib>
#include <vector>

#include <zlib.h>
#include <bzlib.h>

#include "decoder.hpp"
#include "lvltwodef.hpp"


int Decoder::DecodeHeader(ArchiveFile &archive, std::unique_ptr<volume_header> &header){
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
	// skip "."
	archive.ignore(1);
	uint8_t version = static_cast<uint8_t>(std::stoi(version_raw));

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


	header->version = version;
	header->extension_num = ext_num;
	header->date = date;
	header->time = time;
	header->icao = std::string(icao);

	return 0;
}

int Decoder::DecodeMetadata(ArchiveFile &archive, std::unique_ptr<metadata_record> &metadata){
	// Metadata record always 325888 bytes exactly
	if(archive.read(metadata->data, 325888) < 325888){
		std::cerr << "Metadata record less than standard 325888 bytes." << std::endl;
		return -1;
	}	

	//TODO: Add parsing logic for metadata messages (not top priority)

	return 0;
}

int Decoder::DecodeMessages(ArchiveFile &archive, archive_file &file){
	if(archive.at_end()){
		std::cerr << "Unexpected EOF. Archive is header only." << std::endl;
		return -1;
	}

	// Parse messages until EOF
	uint64_t message_qty = 0;
	while(!archive.at_end()){
		// Skip 12 bytes of zeros prepended to all messages (still don't get this)
		if(!archive.ignore(12))
			return 0; // I don't think this is an error atm (or even possible)
		
		uint32_t message_size;
		uint16_t message_size_read;
		uint8_t rda_channel, message_type;
		if((archive.readIntegral(message_size_read)<2)
			|| (archive.readIntegral(rda_channel)<1)
			|| (archive.readIntegral(message_type)<1)
			|| (!archive.ignore(8))){
			std::cerr << "Error parsing message header: Message #" << message_qty << std::endl;
			return -1;
		}	

		// Case where message size > 65535 halfwords 
		if(message_size_read == 65535){
			uint16_t most_sig_halfword, least_sig_halfword;
			archive.readIntegral(most_sig_halfword);
			archive.readIntegral(least_sig_halfword);
			message_size = (most_sig_halfword << 16) | least_sig_halfword;
		}
		else
			message_size = message_size_read*2; // multiply 2 for halfword->byte conversion

		switch(message_type){
			case MESSAGE_TYPE_31:
				//...
				break;
			default:
				std::cerr << "Message Type: " << message_type << " not handled (Message # " << message_qty 
					<< ")" << std::endl;
				break;
		}

	}

	return 0;
}

int Decoder::DecodeArchive(const std::string &file_name, const bool &dump, archive_file &file){
	Decoder::ArchiveFile archive(file_name);
	
	if(dump){
	 	std::string dump_name = "DECOMP_" + file_name;
		archive.dump_to_file(dump_name);
	}

	// Parse volume header
	file.header = std::make_unique<volume_header>();
	if(Decoder::DecodeHeader(archive, file.header) < 0)
		return -1;

	// "Parse" metadata record
	file.metadata = std::make_unique<metadata_record>();
	if(Decoder::DecodeMetadata(archive, file.metadata))
		return -1;

	// Parse all messages remaining
	if(Decoder::DecodeMessages(archive, file) < 0)
		return -1;

	if(!archive.at_end()){
		std::cerr << "Unexpected non EOF. Decode attempt success unknown. Archive file may be corrupt." << std::endl;
		return -2;
	}

	return 0;
}

