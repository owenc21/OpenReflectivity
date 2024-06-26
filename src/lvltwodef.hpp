/**
 * @file lvltwodef.hpp
 * @brief Header file for constants and typedefs for NEXRAD Level II data
 * @author Owen Capell
*/

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <array>
#include <variant>	

enum class MomentType {REF, VEL, SW};

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

// TODO: add intelligent parsing
typedef struct {
	uint8_t data[325888];
} metadata_record;

/**
 * @struct
 * @brief A struct to hold information about gates of a specific data moment type
 * @member moment
 * Member 'moment' is a MomentType indicating the data moment type of the radial data
 * @member num_gates
 * Member 'num_gates' is an integer denoting the number of gates in the radial
 * @member ctrl_flags
 * Member 'ctrl_flags' is a integer denoting the control flags
 * @member range
 * Member 'range' is a float denoting the range (km) to first gate
 * @member range_interval
 * Member 'range_interval' is a float denoting the interval (km) between gates
 * @member snr
 * Member 'snr' is a float indicating the signal to noise ratio
 * @member scale
 * Member 'scale' is a float denoting the scale used in translating real values to recorded values
 * @member offset
 * Member 'offset' is a float denoting the offset ued in translating real values to recorded values
 * @member word_size
 * Member word_size is a bool indicating whether 8-bit (false) words are used for data or 16-bit (true)
 * @member data
 * Member data is a vector of floats corresponding to (sequentially) the gates (converted; true values) of the moment type
 */
typedef struct {
	MomentType moment;
	uint16_t num_gates;
	uint8_t ctrl_flags;
	float range;
	float range_interval;
	float snr;
	float scale;
	float offset;
	bool word_size;
	std::vector<float> data;
} radial;

/**
 * @struct
 * @brief A struct to hold information about a radial
 * @member azimuth
 * Member 'azimuth' is a float denoting the azimuth angle of the radial
 * @member azimuth_num
 * Member 'azimuth_num' is an integer denoting which (index) of the azimuth angle
 * @member radial_length
 * Member 'radial_length' is an integer denoting length of radial in bytes
 * @member radial_status
 * Member 'radial_status' is an integer denoting the radial status
 * @member num_data_blocks
 * Member 'num_data_block' is an integer denoting how many data blocks (between 4-10) in the radial
 * @member ptr_x_y
 * Member 'ptr_x_y' is a pointer to x product/property and y denotes either constant or data block
 * @member azimuth_spacing
 * Member 'azimuth_spacing' is a bool denoting azimuth spacing resolution (true=1.0, false=0.5)
 * @member ref
 * Member 'ref' is a std::unique_ptr to a radial struct to hold information about the reflectivity gates of the radial
 */
typedef struct {
	float azimuth;
	uint16_t azimuth_num;
	uint16_t radial_length;
	uint16_t radial_status;
	uint8_t num_data_blocks;
	uint32_t ptr_vol_const;
	uint32_t ptr_elv_const;
	uint32_t ptr_rad_const;
	uint32_t ptr_ref_block;
	uint32_t ptr_vel_block;
	uint32_t ptr_sw_block;
	uint32_t ptr_zdr_block;
	uint32_t ptr_phi_block;
	uint32_t ptr_rho_block;
	uint32_t ptr_cfp_block;	
	bool azimuth_spacing;
	std::unique_ptr<radial> ref;
} radial_data;

/**
 * @struct
 * @brief A struct to hold all radials of a given elevation
 * @member elevation
 * Member 'elevation' is a float denoting the elevation angle
 * @member elevation_num
 * Member 'elevation_num' is an integer denoting the index of elevation (which elevation)
 * @member radials
 * Member 'radials' is a vector of radial_data structs, containing information about each radial
 */
typedef struct {
	float elevation;
	uint8_t elevation_num;
	std::vector<radial_data> radials;
} elevation_head;

/**
 * @struct
 * @brief A struct to hold all relevant information from the NEXRAD Level II archive file
 * @member header
 * Member 'header' is a volume_header struct to hold information about the volume header 
 * @member metadata
 * Member 'metadata' is a metadata_record struct to hold information about the volume metadata
*/
typedef struct{
	std::unique_ptr<volume_header> header;
	std::unique_ptr<metadata_record> metadata;
	std::array<std::shared_ptr<elevation_head>, 33> scan_elevations;
} archive_file;

constexpr size_t BZIP2_DECOMPRESS_BUFSIZE = 1000000;

constexpr uint8_t MESSAGE_TYPE_31 = 31;