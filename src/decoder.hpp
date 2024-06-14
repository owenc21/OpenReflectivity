/**
 * @file decoder.hpp
 * @brief Header file for decoding utility functions, constants, and typedefinitions
 * @author Owen Capell
*/

#pragma once

#include <string>
#include <fstream>
#include <vector>

#include "lvltwodef.hpp"

/**
 * @namespace Decoder
 * @brief Encapsulate decoding functions
*/
namespace Decoder
{
	/**
	 * @class ArchiveFile
	 * @brief A class that decompresses a level 2 archive file and acts as a stream for the uncompressed data
	*/
	class ArchiveFile{
	private:
		bool initialized;
		std::vector<uint8_t> data; 
		uint64_t pointer;

		/**
		 * @brief Decompresses the entire file (if Gzip compressed) into a given out vector
		 * @param file_name	A string representing the file name of the archive file
		 * @param out	A reference to a vector to store the decompressed file (bytes)
		 * @return 0 on success, -1 on any error
		*/
		int decompressGzip(const std::string &file_name, std::vector<uint8_t> &out);

		/**
		 * @brief Decompresses a block of Bzip2 compressed data to a given out vector
		 * @param compressed_block	Pointer to a buffer of compressed data
		 * @param size	Size of compressed block in bytes
		 * @param out	A reference to a vector to store the decompressed block (bytes)
		 * @return 0 on success, -1 on any error
		*/
		int decompressBzip2(const uint8_t *compressed_block, size_t size, std::vector<uint8_t> &out);

	public:
		/**
		 * @brief Tells whether object is initialized
		 * @returns Internal boolean initialization flag
		*/
		bool isInitialized(){ return initialized; }

	  	/**
		 * @brief Constructor
		 * @param file_name String representing name of archive file
		*/
	 	ArchiveFile(const std::string &file_name);

		/**
		 * @brief Reads size number of bytes into buffer, starting from internal pointer
		 * @param buffer Pointer to a uint8_t buffer of data (assumed to be big enough)
		 * @param size Number of bytes to read into buffer
		 * @return Number of bytes read into buffer
		*/
		size_t read(uint8_t* buffer, size_t size);

		/**
		 * @brief Reads size number of bytes into buffer, starting from internal pointer
		 * @param buffer Pointer to a char buffer of data (assumed to be big enough)
		 * @param size Number of bytes to read into buffer
		 * @return Number of bytes read into buffer
		*/
		size_t read(char* buffer, size_t size);

		/**
		 * @brief Skips over a given number of bytes by moving the internal pointer by that amount
		 * @param off Number of bytes to skip
		*/
		void ignore(uint64_t off);

		/**
		 * @brief Resets the position of the internal pointer
		 * @param pos New position
		*/
		void seek(uint64_t pos);

		/**
		 * @brief Dumps (binary) contents to a file of the given name
		 * @param file_name Name of the file to dump contents to
		*/
		void dump_to_file(const std::string &file_name);
	};

	/**
	 * @brief Decodes a NEXRAD Level 2 archive file, decompressing if necessary
	 * @param file_name	Name of the NEXRAD Level 2 archieve file
	 * @param file	A reference of an archive_file struct to hold data from archive file
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
