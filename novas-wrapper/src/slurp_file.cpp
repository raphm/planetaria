
#include "slurp_file.h"

#include <cstring>
#include <vector>
#include <fstream>

std::string slurpfile (const std::string& fileName) {

	std::ifstream ifs (fileName.c_str (), std::ios::in | std::ios::binary);

	if (!ifs.is_open ()) {
		std::string errstr (strerror (errno));
		throw std::runtime_error ("error while opening file: " + errstr + " " + fileName);
	}

	// https://stackoverflow.com/questions/22984956/tellg-function-give-wrong-size-of-file

	ifs.ignore (std::numeric_limits<std::streamsize>::max ());
	std::streamsize length = ifs.gcount ();
	ifs.clear ();   //  Since ignore will have set eof.
	ifs.seekg (0, std::ios_base::beg);

	std::vector<char> bytes (length);
	ifs.read (bytes.data (), length);

	if (ifs.bad ()) {
		std::string errstr (strerror (errno));
		throw std::runtime_error ("error while reading file: " + errstr);
	}

	return std::string (bytes.data (), length);
}
