#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <semaphore.h>
#define THREAD_NUM 12

using namespace std;

//Usage: node of the tree

struct parameters{
	vector<double> counts;	//read only,not write
	//vector<vector<double>> *gains_ptr;		//also write , so I use a pointer
	vector<vector<double>> dataset;
	int thread_id;
	int class_position;
	double entropy;
	string mode;
	int i;
	vector<int> selectedFeature;
	vector<vector<double>> &gains;
	parameters (vector<vector<double>> &arg1):gains(arg1) {}
};


sem_t mutex;

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


void *calculate_function(void *param1){
	struct parameters *param = (struct parameters*) param1;
	for (int j=param->thread_id; j < param->dataset.size()-1; j+=THREAD_NUM)
		{
			if (param->dataset[j][param->class_position] == param->dataset[j + 1][param->class_position]) continue;

			//Split data into lower part and higher part according to the class changed location (not going to get gap value)
			vector<vector<double>> lowData(param->dataset.begin(), param->dataset.begin() + j + 1);
			vector<vector<double>> highData(param->dataset.begin() + j + 1, param->dataset.end());
			
			//Calculate each classes' percentage for lower part and higher part
			vector<double> lowCount, highCount;
			lowCount.resize(param->counts.size());
			highCount.resize(param->counts.size());
			for (int k = 0; k < param->counts.size(); k++)
			{
				lowCount[k] = 0.0;
				highCount[k] = 0.0;
			}
			for (int k = 0; k < lowData.size(); k++)
			{
				lowCount[lowData[k][param->class_position]] += 1.0;
			}
			for (int k = 0; k < highData.size(); k++)
			{
				highCount[highData[k][param->class_position]] += 1.0;
			}
			for (int k = 0; k < param->counts.size(); k++)
			{
				lowCount[k] /= (double)lowData.size();
				highCount[k] /= (double)highData.size();
			}

			//Calculate lower and higher Entropy (or gini)
			double lowEntropy = 0, highEntropy = 0;
			if (param->mode == "entropy")
			{
				for (int k = 0; k < param->counts.size(); k++)
				{
					if (lowCount[k] != 0) lowEntropy += -(lowCount[k] * log2(lowCount[k]));
					if (highCount[k] != 0) highEntropy += -(highCount[k] * log2(highCount[k]));
				}
			}
			else if (param->mode == "gini")
			{
				for (int k = 0; k < param->counts.size(); k++)
				{
					if (lowCount[k] != 0) lowEntropy += (lowCount[k] * lowCount[k]);
					if (highCount[k] != 0) highEntropy += (highCount[k] * highCount[k]);
				}
				lowEntropy = 1.0 - lowEntropy;
				highEntropy = 1.0 - highEntropy;
			}
			
			//Calculate remainder
			double remainder = ((highData.size() / (double)param->dataset.size()) * highEntropy) + ((lowData.size() / (double)param->dataset.size()) * lowEntropy);
			//Calculate gain
			//printf("end of forloop\n");
			//sem_wait(&mutex);
			param->gains[j][param->selectedFeature[param->i]] = param->entropy - remainder; //Again, gain[split place][feature]
			//sem_post(&mutex);
		}
}

//Usage: Find a best way to split dataset with specific feature
Info CalculateGain(vector<vector<double>> dataset, string mode, vector<int> selectedFeature)
{
	vector<vector<double>> gains;
	gains.clear();
	sem_init (&mutex,0,1);
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
		struct parameters *param[THREAD_NUM];
		pthread_t p_t[THREAD_NUM];

		for(int u=0;u<THREAD_NUM;u++){
		param[u] = new parameters(gains);

		param[u]->counts= counts;
		param[u]-> dataset = dataset;
		param[u]->class_position = classPosition;
		param[u]->entropy = entropy;
		param[u]->mode = mode;
		param[u]->i = i;
		param[u]->selectedFeature = selectedFeature;
		param[u]->thread_id=u;
		pthread_create(&p_t[u],NULL,calculate_function,(void *)param[u]);
		//printf("done pthread_create\n");
		}

		for(int u=0;u<THREAD_NUM;u++){
			pthread_join(p_t[u],NULL);
			delete param[u];
			//printf("done pthread_join\n");
		}
		//delete p_t;
		//printf("delete success");
		//calculate_function(param);
		
	}


	//printf("end of one gain\n");

	/*vector<double> counts_in_thread;	//read only,not write
	vector<vector<double>> *gains;		//also write , so I use a pointer
	vector<vector<double>> dataset;
	int thread_id;
	int class_position
	double entropy;*/

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
struct Node *ConstructTree(vector<vector<double>> dataset, int layer, string mode, vector<int> selectedFeature)
{
	
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
	Info info = CalculateGain(dataset, mode, selectedFeature);

	//Split the dataset into higher and lower part, and complete this node's properties.
	sort(dataset.begin(), dataset.end(), Compare(info.selectedFeature));
	thisNode->gapValue = dataset[info.gapPosition][info.selectedFeature];
	thisNode->selectedFeature = info.selectedFeature;
	thisNode->layer = layer;
	vector<vector<double>> lowerData(dataset.begin(), dataset.begin() + info.gapPosition + 1);
	vector<vector<double>> higherData(dataset.begin() + info.gapPosition + 1, dataset.end());
	thisNode->lowerNode = ConstructTree(lowerData, layer + 1, mode, selectedFeature);
	thisNode->higherNode = ConstructTree(higherData, layer + 1, mode, selectedFeature);

	return thisNode;
	
}