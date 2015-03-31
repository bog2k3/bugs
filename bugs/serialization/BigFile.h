/*
 * BigFile.h
 *
 *  Created on: Mar 23, 2015
 *      Author: bogdan
 */

#ifndef SERIALIZATION_BIGFILE_H_
#define SERIALIZATION_BIGFILE_H_

#include <string>
#include <vector>
#include <map>
#include <cstdlib>

class BinaryStream;

class BigFile {
public:
	struct FileDescriptor {
		size_t size = 0;
		void* pStart = nullptr;
		std::string fileName;

		FileDescriptor() = default;

		FileDescriptor(const FileDescriptor &original)
			: size(original.size), pStart(original.pStart), fileName(original.fileName)
			, ownsMemory_(false) {
		}

		FileDescriptor(size_t size, void* start, std::string const& fileName)
			: size(size), pStart(start), fileName(fileName) {
		}

		~FileDescriptor() {
			if (ownsMemory_)
				free(pStart);
		}
	private:
		bool ownsMemory_ = false;
		friend class BigFile;
	};

	BigFile() = default;
	~BigFile() = default;

	bool loadFromDisk(const std::string &path);
	bool saveToDisk(const std::string &path);

	const FileDescriptor getFile(const std::string &name) const;
	const std::vector<FileDescriptor> getAllFiles() const;

	void addFile(const std::string &filename, const void* buffer, size_t size);
	void extractAll(const std::string &pathOut);
	void extractFile(const std::string &pathOut, const std::string &filename);

private:
	std::map<std::string, FileDescriptor> mapFiles;

	bool loadFromDisk_v1(BinaryStream &fileStream);
	bool saveToDisk_v1(const std::string &path);
};

#endif /* SERIALIZATION_BIGFILE_H_ */
