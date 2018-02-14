/*
 * configFile.h
 *
 *  Created on: Oct 19, 2015
 *      Author: bog
 */

#ifndef CONFIGFILE_H_
#define CONFIGFILE_H_

#include <map>
#include <string>
#include <vector>

bool parseConfigFile(std::string const& filePath, std::map<std::string, std::string> &opts, std::vector<std::string> const& requiredOpts);


#endif /* CONFIGFILE_H_ */
