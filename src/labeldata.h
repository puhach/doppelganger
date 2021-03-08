#ifndef LABELDATA_H
#define LABELDATA_H

#include <map>
#include <string>

// Label -> Name Mapping file
typedef std::map<std::string, std::string> Dict;

Dict generateLabelMap();

#endif	// LABELDATA_H