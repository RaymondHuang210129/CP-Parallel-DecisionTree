#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <algorithm>
#include <iostream>

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
Info calculateGain(vector<vector<double>> dataset)
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

	//Calculate entropy (for whole dataset)
	double entropy = 0.0;
	for (int i = 0; i < counts.size(); i++)
	{
		if (counts[i] != 0.0)  entropy += -(counts[i] * log2(counts[i]));
	}
	
	//Place to store gains [split place][feature]
	vector<vector<double>> gains;
	gains.resize(dataset.size());
	for (int i = 0; i < dataset.size(); i++)
	{
		gains[i].resize(dataset[0].size() - 1);
	}

	//Iterate with different features
	for (int i = 0; i < dataset[0].size() - 1; i++)
	{
		//Sort the dataset with specific feature
		sort(dataset.begin(), dataset.end(), Compare(i));
		
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

			//Calculate lower and higher Entropy
			double lowEntropy = 0, highEntropy = 0;
			for (int k = 0; k < counts.size(); k++)
			{
				if (lowCount[k] != 0) lowEntropy += -(lowCount[k] * log2(lowCount[k]));
				if (highCount[k] != 0) highEntropy += -(highCount[k] * log2(highCount[k]));
			}

			//Calculate remainder
			double remainder = ((highData.size() / (double)dataset.size()) * highEntropy) + ((lowData.size() / (double)dataset.size()) * lowEntropy);
			gains[j][i] = entropy - remainder; //Again, gain[split place][feature]
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
struct Node *constructEntropyTree(vector<vector<double>> dataset, int layer)
{
	cout <<  "-";
	struct Node* thisNode = new Node;

	//Check whether is leaf or not, if yes, return
	thisNode->isLeaf = true;
	for (int i = 1; i < dataset.size(); i++)
	{
		//Check whether last class != this class 
		if (dataset[i][dataset[i].size() - 1] != dataset[i - 1][dataset[i - 1].size() - 1])
		{
			thisNode->isLeaf = false;
			break;
		}
	}
	if (thisNode->isLeaf == true)
	{
		thisNode->leafClass = dataset[0][dataset[0].size() - 1];
		return thisNode;
	}

	//Get the selected feature and split position which have maximum gain
	Info info = calculateGain(dataset);

	//Split the dataset into higher and lower part, and complete this node's properties.
	sort(dataset.begin(), dataset.end(), Compare(info.selectedFeature));
	thisNode->gapValue = dataset[info.gapPosition][info.selectedFeature];
	thisNode->selectedFeature = info.selectedFeature;
	thisNode->layer = layer;
	vector<vector<double>> lowerData(dataset.begin(), dataset.begin() + info.gapPosition + 1);
	vector<vector<double>> higherData(dataset.begin() + info.gapPosition + 1, dataset.end());
	thisNode->lowerNode = constructEntropyTree(lowerData, layer + 1);
	thisNode->higherNode = constructEntropyTree(higherData, layer + 1);

	return thisNode;
	
}