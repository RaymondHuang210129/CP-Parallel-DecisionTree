#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <random>
using namespace std;

DatasetShuffler::DatasetShuffler()
{

}

vector<vector<double>> DatasetShuffler::execute(vector<vector<double>> input)
{
	auto randomGenerator = default_random_engine{};
	shuffle(input.begin(), input.end(), randomGenerator);
	return input;
}