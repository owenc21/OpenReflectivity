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
		message_qty++;
		// Skip 12 bytes of zeros prepended to all messages (still don't get this)
		if(!archive.ignore(12))
			return 0; // I don't think this is an error atm (or even possible)
		
		uint64_t message_start_pos = archive.position();
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

		// Case where message size > 65534 halfwords 
		if(message_size_read == 65535){
			uint16_t most_sig_halfword, least_sig_halfword;
			archive.readIntegral(most_sig_halfword);
			archive.readIntegral(least_sig_halfword);
			message_size = (most_sig_halfword << 16) | least_sig_halfword;
		}
		else
		 	// When message size <= 65534 halfwords, messege seg fields both set to 1
			message_size = message_size_read*2; // multiply 2 for halfword->byte conversion
			// confirming
			uint16_t message_seg1, message_seg2;
			archive.readIntegral(message_seg1);
			archive.readIntegral(message_seg2);
			if(!(message_seg1 == 1 && message_seg2 == 1)){
				std::cerr << "Message with less than 65534 halfwords has improper message segment fields." << std::endl;
			}

		switch(message_type){
			case MESSAGE_TYPE_31:
			 	if(Decoder::Message31::ParseMessage31(archive, file) < 0) return -31;
				break;

			default:
			 	// unsupported message
				// std::cerr << "Message Type: " << static_cast<int>(message_type) << " not handled (Message # " << message_qty 
					// << ")" << std::endl;
				break;
		}

		archive.seek(message_start_pos+message_size);
	}

	return 0;
}

int Decoder::Message31::ParseMessage31(ArchiveFile &archive, archive_file &file){
	uint64_t begin_header_pos = archive.position();

	/* Parse Message31 Header, ignoring fields not used currently*/
	archive.ignore(10);
	
	uint16_t azimuth_num;
	float azimuth_angle;
	
	archive.readIntegral(azimuth_num);
	archive.readFloat(azimuth_angle);

	archive.ignore(2);
	uint32_t ptr_vol_const, ptr_elv_const, ptr_rad_const, ptr_ref_block, ptr_vel_block;
	uint16_t radial_length, radial_length_nh, data_block_count;
	uint8_t elevation_num;
	float elevation_ang;

	archive.readIntegral(radial_length);
	archive.ignore(2);
	archive.readIntegral(elevation_num);
	archive.ignore(1);
	archive.readFloat(elevation_ang);
	archive.ignore(2);
	archive.readIntegral(data_block_count);
	archive.readIntegral(ptr_vol_const);
	archive.readIntegral(ptr_elv_const);
	archive.readIntegral(ptr_rad_const);
	archive.readIntegral(ptr_ref_block);
	// archive.readIntegral(ptr_vel_block);

	if(file.scan_elevations[elevation_num]==nullptr)
		file.scan_elevations[elevation_num] = std::make_shared<elevation_head>();
	std::shared_ptr<elevation_head> elevation = file.scan_elevations[elevation_num];
	elevation->elevation = elevation_ang;
	elevation->elevation_num = elevation_num;

	// Message 31 is one radial with many products... parse them
	std::shared_ptr<radial_data> cur_radial = std::make_shared<radial_data>();
	cur_radial->azimuth = azimuth_angle;
	cur_radial->azimuth_num = azimuth_num;
	cur_radial->num_data_blocks = data_block_count;
	cur_radial->ptr_vol_const = ptr_vol_const;
	cur_radial->ptr_elv_const = ptr_elv_const;
	cur_radial->ptr_rad_const = ptr_rad_const;
	cur_radial->ptr_ref_block = ptr_ref_block;
	Decoder::Message31::ParseRadial(archive, cur_radial, begin_header_pos);
	elevation->radials.push_back(cur_radial);

	return 0;
}

int Decoder::Message31::ParseRadial(ArchiveFile &archive, std::shared_ptr<radial_data> &cur_radial, uint64_t begin_header_pos){
	/* Currently only parsing reflectivity! */

	// REF
	archive.seek(begin_header_pos+cur_radial->ptr_ref_block);
	char type_name[5];
	archive.read(type_name, 4);
	type_name[4] = '\0';
	if(std::string(type_name) != "DREF"){
		std::cerr << "Unable to find \"DREF\" indicator in REF data block." << std::endl; 
		return -1;
	}

	// Reserved
	archive.ignore(4);

	uint16_t num_gates, range_raw, interval_raw, tover_raw;
	short snr_raw;
	float range, interval, tover, snr;
	archive.readIntegral(num_gates);
	archive.readIntegral(range_raw);
	archive.readIntegral(interval_raw);
	archive.readIntegral(tover_raw);
	archive.readIntegral(snr_raw);
	// Scaled (unsigned) integers with 0.001 precision
	range = ((float) range_raw) / 1000;
	interval = ((float) interval_raw) / 1000;
	tover = ((float) tover_raw) / 10; // 0.1 precision
	snr = ((float) snr_raw) / 8; // 0.125 precision
	archive.ignore(1); // control flags

	uint8_t data_word_size;
	float scale, offset;
	archive.readIntegral(data_word_size);
	if(data_word_size != 8){
		std::cerr << "Improper moment word size for REF: (Expected: 8 but got: " << data_word_size << ")" << std::endl;
		return -1;
	}
	archive.readFloat(scale);
	archive.readFloat(offset);

	cur_radial->ref = std::make_unique<radial>();
	cur_radial->ref->moment = MomentType::REF;
	cur_radial->ref->num_gates = num_gates;
	cur_radial->ref->range = range;
	cur_radial->ref->range_interval = interval;
	cur_radial->ref->snr = snr;
	cur_radial->ref->word_size = data_word_size;
	cur_radial->ref->scale = scale;
	cur_radial->ref->offset = offset;

	/* Lambda for converting recorded REF to actual REF */
	auto record_to_true = [scale, offset](uint8_t recorded) { return (((float)recorded) + offset) / scale; };
	
	uint8_t gate;
	for(uint16_t i=0; i<num_gates; i++){
		archive.readIntegral(gate);
		// 0 is below snr, 1 is range folding
		(!(gate == 0 || gate == 1)) ? cur_radial->ref->data.push_back(record_to_true(gate)) : cur_radial->ref->data.push_back(0);
	}

	if(num_gates != cur_radial->ref->data.size()){
		std::cerr << "Discrepancy between number of expected gates (" << num_gates
			<< ") and number of recorded gates(" << cur_radial->ref->data.size() << ")" << std::endl;
		return -1;
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
	if(Decoder::DecodeMetadata(archive, file.metadata) < 0)
		return -1;

	// Initialize all elevation indices to null
	file.scan_elevations.fill(nullptr);

	// Parse all messages remaining
	if(Decoder::DecodeMessages(archive, file) < 0)
		return -1;

	if(!archive.at_end()){
		std::cerr << "Unexpected non-EOF. Decode attempt success unknown. Archive file may be corrupt." << std::endl;
		return -2;
	}

	return 0;
}

