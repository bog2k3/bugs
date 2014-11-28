#ifndef __logger_h__
#define __logger_h__

#include <string>
#include <iostream>

class logger {
public:
	logger(std::string name) : name(name) {}

	void push_prefix(std::string prefix) { this->prefix.push_back(prefix); }
	void pop_prefix() { this->prefix.pop_back(); }

	template<typename T> inline const logger& operator << (T &val) const { std::cout << val; return *this; }
	template<typename T> inline const logger& operator << (T &val) const { writeprefix(); std::cout << val; return *this; }

private:
	std::string name;
	std::vector<std::string> prefix;
	inline void writeprefix() const {
		std::cout << "[" << name;
		for (unsigned i=0, n=prefix.size(); i<n; i++)
			std::cout << "." << prefix[i];
		std::cout << "] ";
	}
};

#endif //__logger_h__
