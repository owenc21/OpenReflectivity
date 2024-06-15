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


int Decoder::ArchiveFile::decompressGzip(const std::string &file_name, std::vector<uint8_t> &out, const bool &gzip){
	std::ifstream file_check(file_name, std::ios::binary);
	if(!file_check.is_open()) return -1;
	
	// Check if Gzip compressed
	unsigned char magic[2];
	file_check.read(reinterpret_cast<char*>(magic), 2);
	file_check.close();

	out.clear();

	std::ifstream file(file_name, std::ios::binary | std::ios::ate);
	if(!file.is_open()) return -1;
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	
	// Gzip compressed
	if(magic[0] == 0x1f && magic[1] == 0x8b && gzip){
		std::vector<char> buffer(size);
		if(!file.read(buffer.data(), size)) return -1;

		z_stream stream = {};
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = size;
		stream.next_in = reinterpret_cast<Bytef*>(buffer.data());

		int ret = inflateInit2(&stream, 16+MAX_WBITS);
		if(ret != Z_OK) return -1;

		// Decompression loop
		std::vector<uint8_t> copy_buf(4096);
		while(ret != Z_STREAM_END){
			stream.avail_out = copy_buf.size();
			stream.next_out = copy_buf.data();

			ret = inflate(&stream, Z_NO_FLUSH);
			if(out.size() < stream.total_out){
				out.insert(out.end(), copy_buf.begin(), copy_buf.begin() + (copy_buf.size() - stream.avail_out));
			}
		}

		inflateEnd(&stream);	
		return ret == Z_STREAM_END;
	}
	else{
		out.resize(size);
		file.read(reinterpret_cast<char*>(out.data()), size);
		return file.good();
	}
}

int Decoder::ArchiveFile::decompressBzip2(const uint8_t *compressed_block, size_t size, std::vector<uint8_t> &out){
	bz_stream stream = {};
	stream.next_in = reinterpret_cast<char*>(const_cast<uint8_t*>(compressed_block));
	stream.avail_in = size;

	out.clear();

	size_t buf_size = BZIP2_DECOMPRESS_BUFSIZE;
	out.resize(buf_size);

	stream.next_out = reinterpret_cast<char*>(out.data());
	stream.avail_out = buf_size;

	if(BZ2_bzDecompressInit(&stream, 0, 0) != BZ_OK){
		std::cerr << "Error initializing bzlib decompression stream." << std::endl;
		return -1;
	}

	// Decompress entire block
	int bzerr;
	while(true){
		bzerr = BZ2_bzDecompress(&stream);
		out.resize(stream.total_out_lo32);

		if(bzerr == BZ_STREAM_END) break;

		// If out of buffer space add another bufsize
		if(stream.avail_out == 0){
			size_t cur_size = out.size();
			out.resize(cur_size + BZIP2_DECOMPRESS_BUFSIZE);
			stream.next_out = reinterpret_cast<char*>(out.data()) + cur_size;
			stream.avail_out = cur_size;
		}	
	}

	BZ2_bzDecompressEnd(&stream);
	return 0;
}

void Decoder::ArchiveFile::ignore(uint64_t off){
	if(!initialized) return;
	uint64_t new_pos = off + pointer;
	if(new_pos >= 0 && new_pos < data.size()) pointer = new_pos;
}

size_t Decoder::ArchiveFile::read(uint8_t* buffer, size_t size){
	if(!initialized) return 0;

	size_t bytes_read = 0;

	while(bytes_read < size && pointer < data.size()){
		*(buffer+bytes_read) = data[pointer];
		pointer++;
		bytes_read++;
	}

	return bytes_read;
}

size_t Decoder::ArchiveFile::read(char* buffer, size_t size){
	return read(reinterpret_cast<uint8_t*>(buffer), size);
}

void Decoder::ArchiveFile::seek(uint64_t pos){
	if(!initialized) return;
	if(pos >= 0 && pos < data.size()) pointer = pos;
}

void Decoder::ArchiveFile::dump_to_file(const std::string &file_name){
	if(!initialized) return;
	const std::vector<uint8_t> data_buffer(data);
	std::ofstream out(file_name, std::ios::out | std::ios::binary);
	out.write(reinterpret_cast<const char*>(data_buffer.data()), data_buffer.size());
	out.close();
}

Decoder::ArchiveFile::ArchiveFile(const std::string &file_name, const bool &gzip, const bool &bzip){
	initialized = false;
	// Decompress entire file (returns original file in vector if uncompressed) into vector
	std::vector<uint8_t> post_gzip;
	if(decompressGzip(file_name, post_gzip, gzip) < 0) return;

	if(!bzip){
		data.reserve(post_gzip.size());
		data.insert(data.end(), post_gzip.begin(), post_gzip.end());
		pointer = 0;
		initialized = true;
		return;
	}

	// Search for BZip2 compressed blocks
	// This could be made quicker by identifying where the blocks are located and then doing the decompression in parallel
	for(uint64_t i=0; i<post_gzip.size()-3; i++){
		// format for block is 4-byte signed integer of size of block, BZhx where x is the compression block size. the compressed block follows
		if(post_gzip[i] == 'B' && post_gzip[i+1] == 'Z' && post_gzip[i+2] == 'h' && post_gzip[i+3] >= '1' && post_gzip[i+3] <= '9'){
			std::vector<uint8_t> decompressed_block;
			int compressed_size(
				(post_gzip[i-4] << 24) |
				(post_gzip[i-3] << 16) |
				(post_gzip[i-2] << 8) |
				post_gzip[i-1]
			);

			size_t size = static_cast<size_t>(abs(compressed_size));
			if(decompressBzip2(&post_gzip[i], size, decompressed_block) < 0) return;

			// Append decompressed block to data but remove the compressed size
			data.resize(data.size()-4);
			data.insert(data.end(), decompressed_block.begin(), decompressed_block.end());
			data.shrink_to_fit();
			i += (size + 1);
		}
		else{
			data.push_back(post_gzip[i]);
		}
	}

	pointer = 0;
	initialized = true;
}

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

int Decoder::DecodeArchive(const std::string &file_name, archive_file &file){
	// Decoder::ArchiveFile archive(file_name);
	Decoder::ArchiveFile archive(file_name, false, true);
	
	archive.dump_to_file("DECOMP");

	// Parse volume header
	file.header = std::make_unique<volume_header>();
	Decoder::DecodeHeader(archive, file.header);

	return 0;
}

