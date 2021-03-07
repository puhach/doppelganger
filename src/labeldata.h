#ifndef LABELDATA_H
#define LABELDATA_H

// label data
// labelData.h
#include <map>
#include <string>
//#include <iostream>

// Label -> Name Mapping file
typedef std::map<std::string, std::string> Dict;
//const Dict& generateLabelMap(void);
Dict generateLabelMap();

#endif	// LABELDATA_H