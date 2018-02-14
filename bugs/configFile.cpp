/*
 * configFile.cpp
 *
 *  Created on: Oct 2, 2015
 *      Author: bog
 */

#include "configFile.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

bool parseConfigFile(std::string const& filePath, std::map<std::string, std::string> &opts, std::vector<std::string> const& requiredOpts) {
	std::ifstream f(filePath);
	if (!f.is_open())
		return false;

	std::cout << "reading config file\"" << filePath << "\"...\n";

	std::string line;
	try {
		while (std::getline(f, line)) {
			if (line.empty() || line[0] == '#')  // ignore comments and empty lines
				continue;

			std::stringstream ss(line);

			std::string tokenName, equalSign, value;
			ss >> tokenName >> equalSign;
			std::ws(ss);	// skip whitespace after =
			std::getline(ss, value);

			if (equalSign != "=") {
				std::cerr << "Invalid line in config file:\n\t\"" << line << "\"";
			}

			opts[tokenName] = value;
		}
		std::cout << "finished parsing config file.\n";
	} catch (std::runtime_error &err) {
		std::cerr << "Reading config file \"" << filePath <<"\"\n" << err.what() << "\n";
	}

	for (auto o : requiredOpts) {
		if (opts.find(o) == opts.end()) {
			std::cerr << "Missing key \"" << o << "\" in config file !!!\n";
			return false;
		}
	}
	return true;
}


