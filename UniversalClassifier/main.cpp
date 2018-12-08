#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "Header.h"
#include <time.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <mpi.h>

#define SEND_CONVERTED_INPUT 1
#define SEND_DATASET_DIMENSION 2
#define SEND_RESULT 3
#define SEND_RESULT_DIMENSION 4
#define SEND_NUMCLASS 5

using namespace std;



double* ConvertVectorToArray(vector<vector<double>> dataset, int numSize, int numAttribute)
{
	double* temp = new double[numSize * numAttribute];
	for (int i = 0; i < numSize; i++)
	{
		for (int j = 0; j < numAttribute; j++)
		{
			temp[i * numAttribute + j] = dataset[i][j];
		}
	}
	return temp;
}

vector<vector<double>> ConvertArrayToVector(double* datasetArray, int numSize, int numAttribute)
{
	vector<vector<double>> temp;
	temp.resize(numSize);
	for (int i = 0; i < numSize; i++)
	{
		temp[i].resize(numAttribute);
		for (int j = 0; j < numAttribute; j++)
		{
			temp[i][j] = datasetArray[i * numAttribute + j];
		}
	}
	return temp;
}


int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	int mpiRank, mpiSize;
	MPI_Status mpiStatus;
	MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
	
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
	vector<vector<double>> statistic;
	statistic.reserve(5000);
	vector<vector<double>> result;
	result.reserve(5000);
	string buffer;
	ifstream input;
	input.open(argv[1]);
	int numClass;

	if (mpiRank == 0)
	{
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
		for (int i = 0; getline(input, buffer, '\n'); i++)
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
		numClass = dataTypeConverter.size(convertedInput[0].size() - 1);
		cout << "#        > " << numClass << " kind of classes" << endl;
		cout << "#        > Execution time: " << chrono::duration_cast<chrono::microseconds>(t3 - t2).count() / (double)1000000 << " sec" << endl;
		//Shuffle the dataset
		cout << "#     Shuffling dataset." << endl;
		DatasetShuffler datasetShuffler;
		convertedInput = datasetShuffler.execute(convertedInput);
		cout << "#     Send dataset to other nodes." << endl;

		

		//Convert 2d vector into 2d array and broadcast to other node
		int datasetDimension[2] = { convertedInput.size(), convertedInput[0].size() };
		//cout << convertedInput.size() << convertedInput[0].size() << endl;
		double* datasetArray = ConvertVectorToArray(convertedInput, datasetDimension[0], datasetDimension[1]);
		for (int i = 1; i < mpiSize; i++)
		{
			MPI_Send(datasetDimension, 2, MPI_INT, i, SEND_DATASET_DIMENSION, MPI_COMM_WORLD);
			MPI_Send(datasetArray, datasetDimension[0] * datasetDimension[1], MPI_DOUBLE, i, SEND_CONVERTED_INPUT, MPI_COMM_WORLD);
		}
		//Split dataset into training dataset and validating dataset
		trainDataset.assign(convertedInput.begin(), convertedInput.begin() + (convertedInput.size() / 2));
		validationDataset.assign(convertedInput.begin() + (convertedInput.size() / 2), convertedInput.end());

		//Construct random forest
		cout << "#     Constructing random forest with 64 trees and randomly select sqrt(#) features each." << endl;
		cout << "#";
		auto t4 = chrono::high_resolution_clock::now();

		vector<Node*>  trees = RandomForestCreater(trainDataset, mpiSize, mpiRank);
		
		auto t5 = chrono::high_resolution_clock::now();
		cout << endl;
		cout << "#        > Execution time: " << chrono::duration_cast<chrono::microseconds>(t5 - t4).count() / (double)1000000 << " sec" << endl;
		//Validate tree
		for (int i = 1; i < mpiSize; i++)
		{
			MPI_Send(&numClass, 1, MPI_INT, i, SEND_NUMCLASS, MPI_COMM_WORLD);
		}
		cout << "#     Validating." << endl;
		Validator validator;
		statistic = validator.getResult(trees, validationDataset, mpiSize, mpiRank, dataTypeConverter.size(validationDataset[0].size() - 1));
		auto t6 = chrono::high_resolution_clock::now();
		cout << "#        > Execution time: " << chrono::duration_cast<chrono::microseconds>(t6 - t5).count() / (double)1000000 << " sec" << endl;


		//receive each result from other nodes, combine them
		cout << "#     Receive result from other node and calculate." << endl;
		int resultDimension[2] = { statistic.size(), statistic[0].size() };
		for (int i = 1; i < mpiSize; i++)
		{
			double* buffArray = new double[resultDimension[0] * resultDimension[1]];
			MPI_Recv(buffArray, resultDimension[0] * resultDimension[1], MPI_DOUBLE, i, SEND_RESULT, MPI_COMM_WORLD, &mpiStatus);
			vector<vector<double>> buff = ConvertArrayToVector(buffArray, resultDimension[0], resultDimension[1]);
			for (int j = 0; j < statistic.size(); j++)
			{
				transform(statistic[i].begin(), statistic[i].end(), buff[i].begin(), statistic[i].begin(), plus<double>());
			}
		}

		//calculate the vote
		result.resize(validationDataset.size());
		for (int i = 0; i < validationDataset.size(); i++)
		{
			//{predict, fact}
			result[i] = { (double)distance(statistic[i].begin(), max_element(statistic[i].begin(), statistic[i].end())), validationDataset[i][validationDataset[0].size() - 1] };
		}
		

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
	}
	else
	{
		//receive dataset from broadcast and convert into vector
		int datasetDimension[2];
		MPI_Recv(datasetDimension, 2, MPI_INT, 0, SEND_DATASET_DIMENSION, MPI_COMM_WORLD, &mpiStatus);
		double* datasetArray = new double[datasetDimension[0] * datasetDimension[1]];
		MPI_Recv(datasetArray, datasetDimension[0] * datasetDimension[1], MPI_DOUBLE, 0, SEND_CONVERTED_INPUT, MPI_COMM_WORLD, &mpiStatus);
		convertedInput = ConvertArrayToVector(datasetArray, datasetDimension[0], datasetDimension[1]);
		
		//Split dataset into training dataset and validating dataset
		trainDataset.assign(convertedInput.begin(), convertedInput.begin() + (convertedInput.size() / 2));
		validationDataset.assign(convertedInput.begin() + (convertedInput.size() / 2), convertedInput.end());

		//Construct random forest
		vector<Node*>  trees = RandomForestCreater(trainDataset, mpiSize, mpiRank);
		//Validate tree
		Validator validator;
		MPI_Recv(&numClass, 1, MPI_INT, 0, SEND_NUMCLASS, MPI_COMM_WORLD, &mpiStatus);
		result = validator.getResult(trees, validationDataset, mpiSize, mpiRank, numClass);

		//Send result to host
		double* resultArray = ConvertVectorToArray(result, result.size(), result[0].size());
		int resultDimension[2] = { result.size(), result[0].size() };
		MPI_Send(resultArray, result.size() * result[0].size(), MPI_DOUBLE, 0, SEND_RESULT, MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return 0;
}

