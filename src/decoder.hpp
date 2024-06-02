/**
 * @file decoder.hpp
 * @brief Header file for decoding utility functions, constants, and typedefinitions
 * @author Owen Capell
*/

#pragma once

#include <string>
#include <fstream>

/**
 * @namespace Decoder
 * @brief Encapsulate decoding functions and constants
*/
namespace Decoder
{

	/**
	 * @brief Decodes a NEXRAD Level 2 archive file, decompressing if necessary
	 * @param file_name	Name of the NEXRAD Level 2 archieve file
	*/
	int DecodeFile(const std::string& file_name);

	/**
	 * @brief Reads a single element of type T from a binary ifstream.
	 * @tparam T The data type of the element to be read. This type should be trivially copyable.
	 * @param stream Reference to the input file stream from which to read.
	 * @return The data read from the stream as an object of type T
	 */
	template <typename T>
	T readBinary(std::ifstream& stream){
		T data;
		stream.read(reinterpret_cast<char*>(&data), sizeof(T));
		return data;
	}

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
