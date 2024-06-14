/**
 * @file lvltwodef.hpp
 * @brief Header file for constants and typedefs for NEXRAD Level II data
 * @author Owen Capell
*/

#pragma once

#include <string>
#include <memory>

/**
 * @struct volume_header
 * @brief A struct to hold relevant information from the NEXRAD Level II volume header
 * @member version
 * Member 'version' holds the version of the radar data
 * @member extension_num
 * Member 'extension_num' holds the potentially rolled-over number of queued radar data volumes
 * @member date
 * Member 'date' contains the NEXRAD-modified Julian date of the start
 * @member time
 * Member 'time' contans the number of milliseconds past midnight
 * @member icao
 * Member 'icao' is a string containing the radar site icao
*/
typedef struct {
	uint8_t version;
	uint8_t extension_num;
	uint32_t date;
	uint32_t time;
	std::string icao;
} volume_header;

/**
 * @struct
 * @brief A struct to hold all relevant information from the NEXRAD Level II archive file
 * @member header
 * Member 'header' is a volume_header struct to hold information about the volume header 
*/
typedef struct{
	std::unique_ptr<volume_header> header;
} archive_file;

constexpr size_t BZIP2_DECOMPRESS_BUFSIZE = 1000000;