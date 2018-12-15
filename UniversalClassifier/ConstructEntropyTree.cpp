#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <pthread.h>

using namespace std;

//Usage: node of the tree



//Usage: passing values with best gain
struct Info
{
	int gapPosition;
	int selectedFeature;
};

//Usage: costumize the compare function with struct (not sure if thread-safe or not,  but probably)
struct Compare
{
	int position;
	Compare(int position) { this->position = position; }
	bool operator () (vector<double> a, vector<double> b)
	{
		return a[position] < b[position];
	}
};

//Usage: Find a best way to split dataset with specific feature
Info CalculateGain(vector<vector<double>> dataset, string mode, vector<int> selectedFeature)
{
	
	
	//Calculate each classes' count to get percentage (for whole dataset)
	vector<double> counts;
	counts.reserve(5000);
	int classPosition = dataset[0].size() - 1;
	for (int i = 0; i < dataset.size(); i++)
	{
		if (dataset[i][classPosition] >= counts.size())
		{
			//make the counts size able to store the count of highest class value
			counts.resize(dataset[i][classPosition] + 1);
			counts[dataset[i][classPosition]] = 1.0;
			continue;
		}
		counts[dataset[i][classPosition]] += 1.0;
	}
	//counts.size() become the class types' total count
	for (int i = 0; i < counts.size(); i++)
	{
		//Get persentage
		counts[i] /= (double)dataset.size();
	}

	//Calculate entropy or gini(for whole dataset)
	double entropy = 0.0;
	if (mode == "entropy")
	{
		for (int i = 0; i < counts.size(); i++)
		{
			if (counts[i] != 0.0)  entropy += -(counts[i] * log2(counts[i]));
		}
	}
	else if (mode == "gini")
	{
		for (int i = 0; i < counts.size(); i++)
		{
			if (counts[i] != 0.0)  entropy += (counts[i] * counts[i]);
		}
		entropy = 1.0 - entropy;
	}
	
	
	//Place to store gains [split place][feature]
	vector<vector<double>> gains;
	gains.resize(dataset.size());
	for (int i = 0; i < dataset.size(); i++)
	{
		gains[i].resize(dataset[0].size() - 1);
	}

	//Iterate with different features
	for (int i = 0; i < selectedFeature.size(); i++)
	{
		//Sort the dataset with specific feature
		sort(dataset.begin(), dataset.end(), Compare(selectedFeature[i]));
		
		//Iterate with different changed points
		for (int j = 0; j < dataset.size() - 1; j++)
		{
			if (dataset[j][classPosition] == dataset[j + 1][classPosition]) continue;

			//Split data into lower part and higher part according to the class changed location (not going to get gap value)
			vector<vector<double>> lowData(dataset.begin(), dataset.begin() + j + 1);
			vector<vector<double>> highData(dataset.begin() + j + 1, dataset.end());
			
			//Calculate each classes' percentage for lower part and higher part
			vector<double> lowCount, highCount;
			lowCount.resize(counts.size());
			highCount.resize(counts.size());
			for (int k = 0; k < counts.size(); k++)
			{
				lowCount[k] = 0.0;
				highCount[k] = 0.0;
			}
			for (int k = 0; k < lowData.size(); k++)
			{
				lowCount[lowData[k][classPosition]] += 1.0;
			}
			for (int k = 0; k < highData.size(); k++)
			{
				highCount[highData[k][classPosition]] += 1.0;
			}
			for (int k = 0; k < counts.size(); k++)
			{
				lowCount[k] /= (double)lowData.size();
				highCount[k] /= (double)highData.size();
			}

			//Calculate lower and higher Entropy (or gini)
			double lowEntropy = 0, highEntropy = 0;
			if (mode == "entropy")
			{
				for (int k = 0; k < counts.size(); k++)
				{
					if (lowCount[k] != 0) lowEntropy += -(lowCount[k] * log2(lowCount[k]));
					if (highCount[k] != 0) highEntropy += -(highCount[k] * log2(highCount[k]));
				}
			}
			else if (mode == "gini")
			{
				for (int k = 0; k < counts.size(); k++)
				{
					if (lowCount[k] != 0) lowEntropy += (lowCount[k] * lowCount[k]);
					if (highCount[k] != 0) highEntropy += (highCount[k] * highCount[k]);
				}
				lowEntropy = 1.0 - lowEntropy;
				highEntropy = 1.0 - highEntropy;
			}
			
			//Calculate remainder
			double remainder = ((highData.size() / (double)dataset.size()) * highEntropy) + ((lowData.size() / (double)dataset.size()) * lowEntropy);
			//Calculate gain
			gains[j][selectedFeature[i]] = entropy - remainder; //Again, gain[split place][feature]
		}
	}

	//Find the maximum gain then get it's feature, gap position and gap value
	struct Info info;
	double maxGain = 0.0;
	for (int i = 0; i < gains.size(); i++)
	{
		for (int j = 0; j < gains[0].size(); j++)
		{
			if (gains[i][j] > maxGain)
			{
				maxGain = gains[i][j];
				info.selectedFeature = j;
				info.gapPosition = i;
			}
		}
	}
	
	return info;
}


//
void *ConstructTree(struct parameters param)
{
	
	struct Node* thisNode = new Node;
	struct parameters Left,Right; 

	//Check whether is leaf or not, if yes, return
	thisNode->isLeaf = true;
	for (int i = 1; i < param.dataset.size(); i++)
	{
		//Check whether last class != this class 
		if (param.dataset[i][param.dataset[i].size() - 1] != param.dataset[i - 1][param.dataset[i - 1].size() - 1])
		{
			thisNode->isLeaf = false;
			break;
		}
	}
	if (thisNode->isLeaf == true)
	{
		thisNode->leafClass = param.dataset[0][param.dataset[0].size() - 1];
		return (void *)thisNode;
	}

	//Get the selected feature and split position which have maximum gain
	Info info = CalculateGain(param.dataset, param.mode, param.selectedFeature);

	//Split the dataset into higher and lower part, and complete this node's properties.
	sort(param.dataset.begin(), param.dataset.end(), Compare(info.selectedFeature));
	thisNode->gapValue = param.dataset[info.gapPosition][info.selectedFeature];
	thisNode->selectedFeature = info.selectedFeature;
	thisNode->layer = param.layer;

	vector<vector<double>> higher_data(param.dataset.begin(), param.dataset.begin() + info.gapPosition + 1);
	vector<vector<double>> lower_data(param.dataset.begin() + info.gapPosition + 1, param.dataset.end());


	Left.dataset = lower_data;
	Right.dataset = higher_data;


	Left.layer=Right.layer = param.layer+1;
	Left.mode =Right.mode = param.mode;
	Left.selectedFeature = Right.selectedFeature = param.selectedFeature;

	
	thisNode->lowerNode = (struct Node*)ConstructTree(Left);
	thisNode->higherNode = (struct Node*)ConstructTree(Right);



	return (void *)thisNode;
	
} //vector<vector<double>> dataset, int layer, string mode, vector<int> selectedFeature