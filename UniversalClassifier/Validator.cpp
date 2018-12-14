#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

Validator::Validator()
{

}


vector<vector<double>> Validator::getResult(Node* tree, vector<vector<double>> dataset)
{
	//result: 0->prediction 1->fact
	vector<vector<double>> result;
	result.resize(dataset.size());
#pragma omp parallel for
	for (int i = 0; i < dataset.size(); i++)
	{
		//Start from the root of tree
		Node* currentLocation = tree;
		while (1)
		{
			if (currentLocation->isLeaf)//Reach the leaf
			{
				result[i] = { currentLocation->leafClass, dataset[i][dataset[0].size() - 1] };
				break;
			}
			if (dataset[i][currentLocation->selectedFeature] > currentLocation->gapValue)
				currentLocation = currentLocation->higherNode;
			else
				currentLocation = currentLocation->lowerNode;
		}

	}
	return result;
}