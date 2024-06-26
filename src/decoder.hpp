/**
 * @file decoder.hpp
 * @brief Header file for decoding utility functions, constants, and typedefinitions
 * @author Owen Capell
*/

#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include "lvltwodef.hpp"

/**
 * @namespace Decoder
 * @brief Encapsulate decoding functions
*/
namespace Decoder
{
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

	/**
	 * @class ArchiveFile
	 * @brief A class that decompresses a level 2 archive file and acts as a stream for the uncompressed data
	*/
	class ArchiveFile{
	private:
		bool initialized;
		std::vector<uint8_t> data; 
		uint64_t pointer;
		uint16_t blocks;

		/**
		 * @brief Decompresses the entire file (if Gzip compressed) into a given out vector
		 * @param file_name	A string representing the file name of the archive file
		 * @param out	A reference to a vector to store the decompressed file (bytes)
		 * @return 0 on success, -1 on any error
		*/
		int decompressGzip(const std::string &file_name, std::vector<uint8_t> &out, const bool &gzip);

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
		 * @brief Constructor accepting file name and option of turning off either or both Gzip and Bzip decompression
		 * @param file_name	Name of archive file
		 * @param gzip Whether to attempt to perform Gzip decompression
		 * @param bzip Whether to attempt to perfrom bzip2 decompression
		*/
		ArchiveFile(const std::string &file_name, const bool &gzip, const bool &bzip);
	  	/**
		 * @brief Constructor only accpeting file name, looking for both Gzip and Bzip2 compression
		 * @param file_name String representing name of archive file
		*/
	 	ArchiveFile(const std::string &file_name) : ArchiveFile(file_name, true, true) {}

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
		 * @brief Reads data into an integral type, based on sizeof type
		 * @tparam Integral type (unsigned)
		 * @param buffer Reference to (unsigned) integral type
		 * @return Number of bytes read into reference 
		 */
		template <typename T>
		size_t readIntegral(T &buffer){
			size_t bytes_read = read(reinterpret_cast<uint8_t*>(&buffer), sizeof(T));
			buffer = reverseEndian(buffer);
			return bytes_read;	
		}

		/**
		 * @brief Reads floating point data into a float reference
		 * @param buffer Reference to a float
		 * @return Number of bytes read
		 */
		size_t readFloat(float &buffer);

		/**
		 * @brief Returns the entire buffer of data
		 * @return Data buffer
		*/
		std::vector<uint8_t> getAll(){ return data; }

		/**
		 * @brief Skips over a given number of bytes by moving the internal pointer by that amount
		 * @param off Number of bytes to skip
		 * @returns Boolean indicator of whether byte skipping was successful
		*/
		bool ignore(uint64_t off);

		/**
		 * @brief Moves back a number of bytes by moving internal pointer by specified amount
		 * @param off Number of bytes to move back
		 * @returns Boolean indicator of whether repositioning was successful
		 */
		bool back(uint64_t off);

		/**
		 * @brief Resets the position of the internal pointer
		 * @param pos New position
		 * @returns Boolean indicator of whteher seek was successful
		*/
		bool seek(uint64_t pos);

		/**
		 * @brief Dumps (binary) contents to a file of the given name
		 * @param file_name Name of the file to dump contents to
		*/
		void dump_to_file(const std::string &file_name);

		/**
		 * @brief Tells whether object is initialized
		 * @returns Internal boolean initialization flag
		*/
		bool isInitialized(){ return initialized; }

		/**
		 * @brief Tell whether object is at EOF
		 * @returns Boolean comparison if pointer is at end
		 */
		bool at_end(){ return pointer >= data.size(); }

		/**
		 * @brief Tells object size
		 * @returns Internal data vector size
		 */
		size_t size(){ return data.size(); }

		/**
		 * @brief Tells number of BZIP2 blocks decompressed
		 * @returns Internal count of decomressed BZIP2 blocks
		 */
		uint16_t num_blocks(){ return blocks; }

		/**
		 * @brief Tells the position of the internal byte pointer
		 * @returns Internal pointer position
		 */
		uint64_t position(){ return pointer; }

		/**
		 * @brief Prints out a given number of bytes starting from the internal position
		 * @param amt The number of byte to print
		 */
		void peek(const uint64_t amt);
	};

	/**
	 * @brief Decodes a NEXRAD Level 2 archive file, decompressing if necessary
	 * @param file_name	Name of the NEXRAD Level 2 archieve file
	 * @param dump Whether to dump the decompressed archive file to "./DUMP"
	 * @param file	A reference of an archive_file struct to hold data from archive file
	 * @return	Status of decode attempt. See documentation for reference (TBD)
	*/
	int DecodeArchive(const std::string& file_name, const bool &dump, archive_file &file);

	/**
	 * @brief Decodes a NEXRAD Level 2 archive file header into the given volume_header struct
	 * @param archive A reference to an ArchiveFile object to read from
	 * @param header A reference to a unique_ptr<volume_header> to write decoded header information to
	 * @return Status of decode attempt. See documentation for reference (TBD)
	*/
	int DecodeHeader(ArchiveFile &archive, std::unique_ptr<volume_header> &header);

	/**
	 * @brief Decodes a NEXRAD Level 2 archtive file metadata record into given metadata_record struct
	 * @param archive A reference to an ArchiveFile object to read from
	 * @param header A reference to a unique_ptr<metadata_record> to write decoded information to
	 * @return Status of decode attempt. See documentation for reference (TBD)
	 */
	int DecodeMetadata(ArchiveFile &archive, std::unique_ptr<metadata_record> &metadata);

	/**
	 * @brief Decodes non-metadata messages in archive file, recording data from select messages
	 * @param archive A reference to an ArchiveFile object to read from
	 * @param file A reference to an archive_file struct to write decoded information to
	 * @return Status of decode attempt. See documentation for reference (TBD)
	 */
	int DecodeMessages(ArchiveFile &archive, archive_file &file);

	namespace Message31{	
		/**
		 * @brief Parses Message 31, starting from the message header, then moves
		 * to constant block followed by radial blocks (only REF right now)
		 * @param archive A reference to an ArchiveFile object to read from
		 * @param file A reference to an archive_file struct to write decoded information to
		 * @return Status of decode attempt. See documentation for reference (TBD)
		 */
		int ParseMessage31(ArchiveFile &archive, archive_file &file);

		/**
		 * @brief Parses the radial for all products (only REF right now)
		 * @param archive A reference to an ArchiveFile object ot read from
		 * @param cur_radial A reference to a radial_data object giving information about the current radial (and where to store pared information)
		 * @param begin_header_pos The byte position of the beginning of the Message 31 (non-generic) header
		 */
		int ParseRadial(ArchiveFile &archive, std::shared_ptr<radial_data> &cur_radial, uint64_t begin_header_pos);
	}
}
