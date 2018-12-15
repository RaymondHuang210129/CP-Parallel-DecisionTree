#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <pthread.h>
using namespace std;

//Usage: input: a line, output: string vector
class StringSplitter
{
public:
	StringSplitter();
	vector<string> parseString(string);
private:
	string token;
};

//Usage: input: vector<vector<string>>, output: vector<vector<double>>
class DataTypeConverter
{
public:
	DataTypeConverter();
	vector<vector<double>> convertToDouble(vector<vector<string>>);
private:
	int totalIndex;
	int totalTransition;
	double buffer;
	vector<vector<string>> table;
	vector<vector<double>> convertedInput;
};

//Usage: input: dataset, output: dataset
class DatasetShuffler
{
public:
	DatasetShuffler();
	vector<vector<double>> execute(vector<vector<double>>);
private:

};

class Validator
{
public:
	Validator();
	vector<vector<double>> getResult(struct Node*, vector<vector<double>>);
};

//Usage: recursively constructing tree
void* ConstructTree(void* param);

struct Node
{
	int layer;
	int selectedFeature;
	bool isLeaf;
	double gapValue;
	struct Node* higherNode;
	struct Node* lowerNode;
	double leafClass;
};

struct parameters{
vector<vector<double>> dataset;
int layer;
string mode;
vector<int> selectedFeature;
};

