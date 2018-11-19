#include "stdafx.h"
#include "Header.h"
#include <string>
#include <cstring>
#include <algorithm>
using namespace std;

DataTypeConverter::DataTypeConverter()
{

}

vector<vector<double>> DataTypeConverter::convertToDouble(vector<vector<string>> input)
{
	totalIndex = input[0].size();
	totalTransition = input.size();
	table.resize(totalIndex);

	convertedInput.resize(totalTransition); 
	for (int i = 0; i < totalTransition; i++)
	{
		convertedInput[i].resize(totalIndex);
	}
#pragma omp parallel for
	for (int j = 0; j < totalIndex; j++)
	{
		//using stod() with try and catch to know that whether a value is a number or not
		for (int i = 0; i < totalTransition; i++)
		{
			try
			{
				buffer = stod(input[i][j]);
				//A string is not present
				convertedInput[i][j] = buffer;
			}
			catch (exception& e) //A string is present
			{
				//find the string inside the encode vector and get its index 
				ptrdiff_t pos = distance(table[j].begin(), find(table[j].begin(), table[j].end(), input[i][j])); 
				if (pos >= table[j].size()) //the string value pair has not bind yet 
				{
					//create a new string--value pair with position number in vector
					table[j].push_back(input[i][j]);
					convertedInput[i][j] = (double)pos;
				}
				else
				{
					//get it's position number to be it's value
					convertedInput[i][j] = (double)pos;
				}
			}
		}

	}

	return convertedInput;
}