#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

Validator::Validator()
{

}


vector<vector<double>> Validator::getResult(vector<Node*> trees, vector<vector<double>> dataset)
{
	//result: 0->prediction 1->fact
	vector<vector<double>> result;
	result.resize(dataset.size());

	//vote data structure, initalize to 0 for all classes
	
	
	for (int i = 0; i < dataset.size(); i++)
	{
		vector<int> vote(dataset.size() - 1, 0);
		//Iterate with different trees
		for (int j = 0; j < trees.size(); j++)
		{
			//Start from the root of tree
			Node* currentLocation = trees[j];
			while (1)
			{
				if (currentLocation->isLeaf)//Reach the leaf
				{
					//vote
					vote[currentLocation->leafClass]++;
					break;
				}
				if (dataset[i][currentLocation->selectedFeature] > currentLocation->gapValue)
					currentLocation = currentLocation->higherNode;
				else
					currentLocation = currentLocation->lowerNode;
			}
		}

		//find the class with maximum vote and put into result
		result[i] = { (double)distance(vote.begin(), max_element(vote.begin(), vote.end())), dataset[i][dataset[0].size() - 1] };

	}
	return result;
}