#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

Validator::Validator()
{

}


vector<vector<double>> Validator::getResult(struct Node* tree, vector<vector<double>> dataset)
{
	//result: 0->prediction 1->fact
	vector<vector<double>> result;
	result.resize(dataset.size());
	for (int i = 0; i < dataset.size(); i++)
	{
		//Start from the root of tree
		struct Node* currentLocation = tree;
		while(1)
		{
			if (currentLocation->isLeaf == true)//Reach the leaf
			{
				//Push predicted value and 
				vector<double> buff = { currentLocation->leafClass, dataset[i][dataset[i].size() - 1] };
				result[i]= buff;
				break;
			}
			if (dataset[i][currentLocation->selectedFeature] > currentLocation->gapValue)
			{
				currentLocation = currentLocation->higherNode;
			}
			else
			{
				currentLocation = currentLocation->lowerNode;
			}
		}
	}
	return result;
}