/**
 * @file decoder.hpp
 * @brief Header file for decoding utility functions, constants, and typedefinitions
 * @author Owen Capell
*/

#pragma once

#include <string>
#include <fstream>

#include "lvltwodef.hpp"

/**
 * @namespace Decoder
 * @brief Encapsulate decoding functions and constants
*/
namespace Decoder
{

	/**
	 * @brief Decodes a NEXRAD Level 2 archive file, decompressing if necessary
	 * @param file_name	Name of the NEXRAD Level 2 archieve file
	 * @return	Status of decode attempt. See documentation for reference
	*/
	int DecodeArchive(const std::string& file_name, archive_file &file);

	/**
	 * @brief Reverses the endianness of an arbitrary integral type
	 * @tparam T	The integral (not checked) datatype to reverse
	 * @param data	Reference to the data for which the endianness should be reversed
	 * @return	Copy of data with endianness reveresed	
	*/
	template <typename T>
	T reverseEndian(const T& data){
		size_t num_bytes = sizeof(T);

		T reverse_endian = 0;
		for(int i=0; i<num_bytes; i++){
			reverse_endian |= ((data >> (8 * i)) & 0xFF) << (8 * (num_bytes - 1 - i));
		}

		if(num_bytes == 1) reverse_endian = data;

		return reverse_endian;
	}


}
