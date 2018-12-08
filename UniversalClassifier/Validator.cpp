#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

Validator::Validator()
{

}


vector<vector<double>> Validator::getResult(vector<Node*> trees, vector<vector<double>> dataset, int mpiSize, int mpiRank, int numClass)
{
	//result: 0->prediction 1->fact
	vector<vector<double>> result;

	//vote data structure, initalize to 0 for all classes
	
	for (int i = 0; i < dataset.size(); i++)
	{
		//Init vote vector to 0
		vector<double> vote(numClass, 0);

		//Iterate with different trees
		for (int j = 0; j < trees.size(); j++)
		{
			//Start from the root of tree
			Node* currentLocation = trees[j];
			while (1)
			{
				if (currentLocation->isLeaf)//Reach the leaf
				{
					//vote (position = class)
					//cout << i << " " << mpiRank << " " << currentLocation->leafClass << " " << j << endl;
					vote[currentLocation->leafClass] += 1.0;
					break;
				}
				if (dataset[i][currentLocation->selectedFeature] > currentLocation->gapValue)
					currentLocation = currentLocation->higherNode;
				else
					currentLocation = currentLocation->lowerNode;
			}
		}

		//return vote result (not calculated)
		result.push_back(vote);

	}
	return result;
}