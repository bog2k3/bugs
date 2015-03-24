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

class BigFile {
public:
	struct FileDescriptor {
		size_t size = 0;
		void* pStart = nullptr;
		std::string fileName;

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

	void loadFromDisk(const std::string &path);
	void saveToDisk(const std::string &path);

	FileDescriptor getFile(const std::string &name);
	std::vector<FileDescriptor> getAllFiles();

	void addFile(const std::string &filename, void* buffer, size_t size);
	void extractAll(const std::string &pathOut);
	void extractFile(const std::string &pathOut, const std::string &filename);

private:
	std::map<std::string, FileDescriptor> mapFiles;
};

#endif /* SERIALIZATION_BIGFILE_H_ */
