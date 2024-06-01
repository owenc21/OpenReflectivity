/**
 * @file decoder.hpp
 * @brief Header file for decoding utility functions, constants, and typedefinitions
 * @author Owen Capell
*/

#pragma once

#include <string>

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

}
