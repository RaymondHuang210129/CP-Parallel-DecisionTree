#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "Header.h"
#include <time.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <pthread.h>


using namespace std;


int main(int argc, char *argv[])
{
	vector<string> rawInput; //Raw input lines
	rawInput.reserve(5000);
	vector<vector<string>> splitedInput; // 2D data
	splitedInput.reserve(5000);
	vector<vector<double>> convertedInput; // 'doublized
	convertedInput.reserve(5000);
	vector<vector<double>> trainDataset;
	trainDataset.reserve(5000);
	vector<vector<double>> validationDataset;
	validationDataset.reserve(5000);
	vector<vector<double>> result;
	result.reserve(5000);
	string buffer;
	ifstream input;
	input.open(argv[1]);

	//Print welcome words
	cout << "###################################################################################" << endl;
	cout << "#                                                                                 #" << endl;
	cout << "#   U   U NN  N IIIII V   V EEEEE RRRR   SSSS  AAA  L                             #" << endl;
	cout << "#   U   U N N N   I   V   V E____ R   R S___  A   A L                             #" << endl;
	cout << "#   U   U N  NN   I    V V  E     RRRR      S AAAAA L                             #" << endl;
	cout << "#    UUU  N   N IIIII   V   EEEEE R   R SSSS  A   A LLLLL                         #" << endl;
	cout << "#                                                                                 #" << endl;
	cout << "#                    CCCC L      AAA   SSSS  SSSS IIIII FFFFF IIIII EEEEE RRRR    #" << endl;
	cout << "#                   C     L     A   A S___  S___    I   F       I   E____ R   R   #" << endl;
	cout << "#                   C     L     AAAAA     S     S   I   FFFF    I   E     RRRR    #" << endl;
	cout << "#                    CCCC LLLLL A   A SSSS  SSSS  IIIII F     IIIII EEEEE R   R   #" << endl;
	cout << "#                                                                                 #" << endl;
	cout << "###################################################################################" << endl;
	
	

	//Store the raw input line by line
	cout << "#     Input File: " << argv[1] << endl;
	for ( int i = 0; getline(input, buffer, '\n'); i++)
	{
		rawInput.push_back(buffer);
	}

	//Start the timer
	auto t1 = chrono::high_resolution_clock::now();
	//Split a ine into strings
	cout << "#     Splitting input." << endl;
	StringSplitter stringSplitter;

	for (int i = 0, j = rawInput.size(); i < j; i++)
	{
		splitedInput.push_back(stringSplitter.parseString(rawInput[i]));
	}
	cout << "#        > Dataset contains " << splitedInput.size() << " data and " << splitedInput[0].size() - 1 << " features each" << endl;
	auto t2 = chrono::high_resolution_clock::now();
	//cout << "#        > Execution time: " << (t2 - t1) / (double)(CLOCKS_PER_SEC) << " sec" << endl;
	cout << "#        > Execution time: " << chrono::duration_cast<chrono::microseconds>(t2 - t1).count() / (double)1000000 << " sec" << endl;

	//Convert all values into double, including strings, ints, and double
	cout << "#     Convert all features into double." << endl;
	DataTypeConverter dataTypeConverter;
	convertedInput = dataTypeConverter.convertToDouble(splitedInput);

	auto t3 = chrono::high_resolution_clock::now();
	cout << "#        > Execution time: " << chrono::duration_cast<chrono::microseconds>(t3 - t2).count() / (double)1000000 << " sec" << endl;
	//Shuffle the dataset
	cout << "#     Shuffling dataset." << endl;
	DatasetShuffler datasetShuffler;
	convertedInput = datasetShuffler.execute(convertedInput);

	//Split dataset into training dataset and validating dataset
	trainDataset.assign(convertedInput.begin(), convertedInput.begin() + (convertedInput.size() / 2));
	validationDataset.assign(convertedInput.begin() + (convertedInput.size() / 2), convertedInput.end());

	auto t4 = chrono::high_resolution_clock::now();
	//Construct Decision Tree
	cout << "#     Constructing decision tree." << endl;
	cout << "#";
	vector<int> selectedFeature(trainDataset[0].size());
	for (int i = 0; i < trainDataset[0].size() - 1; i++) selectedFeature[i] = i;

	struct parameters *pa = new parameters;
	pa->dataset = trainDataset;
	pa->layer=0;
	pa->mode = "entropy";
	pa->selectedFeature = selectedFeature;

	Node* tree = (Node *)ConstructTree(pa);

	delete pa;
	//vector<Node*>  trees = RandomForestCreater(trainDataset);
	cout << endl;
	auto t5 = chrono::high_resolution_clock::now();
	cout << "#        > Execution time: " << chrono::duration_cast<chrono::microseconds>(t5 - t4).count() / (double)1000000 << " sec" << endl;

	//Validate tree
	cout << "#     Validating." << endl;
	Validator validator;
	result = validator.getResult(tree, validationDataset);
	auto t6 = chrono::high_resolution_clock::now();
	cout << "#        > Execution time: " << chrono::duration_cast<chrono::microseconds>(t6 - t5).count() / (double)1000000 << " sec" << endl;

	

	//Show result
	cout << "#     Prediction result:" << endl;
	
	double rate = 0.0;
	for (int k = 0; k < validationDataset.size(); k++)
	{
		if (result[k][0] - result[k][1] == 0)
		{
			rate += 1.0;
		}
	}
	rate /= (double)validationDataset.size();
	cout << "#     Classfication +-0 (for classfication): " << rate * 100.0 << " %" << endl;

	rate = 0.0;
	for (int k = 0; k < validationDataset.size(); k++)
	{
		if (fabs(result[k][0] - result[k][1]) <= 1.0)
		{
			rate += 1.0;
		}
	}
	rate /= (double)validationDataset.size();
	cout << "#     Classfication +-1 (for regression-like dataset): " << rate * 100.0 << " %" << endl;

	rate = 0.0;
	for (int k = 0; k < validationDataset.size(); k++)
	{
		if (fabs(result[k][0] - result[k][1]) <= 2.0)
		{
			rate += 1.0;
		}
	}
	rate /= (double)validationDataset.size();
	cout << "#     Classfication +-2 (for regression-like dataset): " << rate * 100.0 << " %" << endl;

	//Show the execution time
	cout << "#     Total execution time: " << chrono::duration_cast<chrono::microseconds>(t6 - t1).count() / (double)1000000 << " sec" << endl;
	//system("pause");

	return 0;
}

