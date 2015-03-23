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

class BigFile {
public:
	struct FileDescriptor {
		unsigned size = 0;
		void* pStart = nullptr;
		std::string fileName;
	};

	enum AccessMode {
		MODE_READ,
		MODE_REWRITE
	};

	BigFile(const std::string &path, AccessMode mode);
	~BigFile();

	FileDescriptor getFile(const std::string &name);
	std::vector<FileDescriptor> getAllFiles();

	void extractAll(const std::string &pathOut);
	void extractFile(const std::string &pathOut, const std::string &filename);

private:
	AccessMode mode_;
	std::map<std::string, FileDescriptor> mapFiles;
};

#endif /* SERIALIZATION_BIGFILE_H_ */
