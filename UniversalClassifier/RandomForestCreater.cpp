#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>

//Create random forest
vector<Node*> RandomForestCreater(vector<vector<double>> dataset, int mpiSize, int mpiRank)
{
	random_device rd;
	default_random_engine gen = default_random_engine(rd());
	
	
	//State the number to be selected for each tree
	int numSelectFeature = sqrt(dataset[0].size() - 1) + 1;
	//State the number of tree to construct
	int numTree = 64;

	vector<Node*> trees;

	//randomly select features (non-duplicate)
	vector<int> features(dataset[0].size() - 1);
	for (int i = 0; i < dataset[0].size() - 1; i++) features[i] = i;

	for (int i = mpiRank; i < numTree; i += mpiSize)
	{
		//generate a list of selected features
		vector<int> features2 = features;
		shuffle(features2.begin(), features2.end(), gen);
		vector<int> selectedFeature(features2.begin(), features2.begin() + numSelectFeature);
		
		//randomly select samples (with duplicate)
		uniform_int_distribution<int> dis(0, dataset.size() - 1);
		vector<vector<double>> selectedSample;
		for (int j = 0; j < dataset.size(); j++)
		{
			selectedSample.push_back(dataset[dis(gen)]);
		}
		cout << "-" << flush;
		trees.push_back(ConstructTree(selectedSample, 0, "entropy", selectedFeature));
	}

	return trees;
}