#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;

StringSplitter::StringSplitter() {}

vector<string> StringSplitter::parseString(string input)
{
	istringstream tokenStream(input);
	vector<string> results;
	//using getline and delimiter ',' to split a line to vector of strings
	while (getline(tokenStream, token, ','))
	{
		results.push_back(token);
	}
	return results;
}