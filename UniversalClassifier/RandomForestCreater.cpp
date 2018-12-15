#include "stdafx.h"
#include "Header.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <semaphore.h>
#include <pthread.h>
#define MAX_CORE 1

pthread_t thread_id[MAX_CORE];

class parameter_for_function{
public:
	vector<int> features;
	int numSelectFeature;
	vector<vector<double>> dataset;
	int id;
	int numTree;
	default_random_engine gen;

	parameter_for_function(vector<int> in_feature , int in_numSelectedFeature , vector<vector<double>> in_dataset,int in_id , int in_numTree,default_random_engine in_gen){
		features=in_feature;
		numSelectFeature = in_numSelectedFeature;
		dataset = in_dataset;
		id = in_id;
		numTree = in_numTree;
		gen=in_gen;
	}
};

sem_t mutex_for_node_vector;
vector<Node*> trees;
void *for_loop_creation(void *);


//Create random forest
vector<Node*> RandomForestCreater(vector<vector<double>> dataset)
{
	random_device rd;
	default_random_engine gen = default_random_engine(rd());
	void *ret;
	
	sem_init(&mutex_for_node_vector,0,1);
	
	//State the number to be selected for each tree
	int numSelectFeature = sqrt(dataset[0].size() - 1) + 1;
	//State the number of tree to construct
	int numTree = 64;

	
	trees.resize(numTree);

	//randomly select features (non-duplicate)
	vector<int> features(dataset[0].size() - 1);
	for (int i = 0; i < dataset[0].size() - 1; i++) features[i] = i;

//#pragma omp parallel for
		parameter_for_function *input_for_function_ptr[MAX_CORE];
		for(int k=0;k<MAX_CORE;k++){
		input_for_function_ptr[k]= new parameter_for_function(features,numSelectFeature,dataset,k,numTree,gen);
		int t=k;
		pthread_create(&thread_id[t], NULL, &for_loop_creation , input_for_function_ptr[t]);	
		}

	/*for (int i = 0; i < numTree; i++)	//input:feature,numselect,feature,dataset
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
		trees[i] = ConstructTree(dataset, 0, "entropy", selectedFeature);
	}*/
		for(int k=0;k<MAX_CORE;k++){
			pthread_join(thread_id[k],&ret);
			delete input_for_function_ptr[k];
		}
		
	return trees;
}


void *for_loop_creation(void *input_for_function_param){
	parameter_for_function * input_for_function=(parameter_for_function *)input_for_function_param;
	//for (int i = ceil(input_for_function->numTree/MAX_CORE)*(input_for_function->id); i < ceil(input_for_function->numTree/MAX_CORE)*(input_for_function->id+1); i++)	//input:feature,numselect,feature,dataset
	for(int i=input_for_function->id; i<(input_for_function->numTree); i+=MAX_CORE)
	{
		//generate a list of selected features
		//printf("%d\t",i);
		vector<int> features2 = input_for_function->features;
		shuffle(features2.begin(), features2.end(), input_for_function->gen);
		vector<int> selectedFeature(features2.begin(), features2.begin() + input_for_function->numSelectFeature);
		
		//randomly select samples (with duplicate)
		uniform_int_distribution<int> dis(0, input_for_function->dataset.size() - 1);
		vector<vector<double>> selectedSample;
		for (int j = 0; j < input_for_function->dataset.size(); j++)
		{
			selectedSample.push_back(input_for_function->dataset[dis(input_for_function->gen)]);
		}
		cout << "-" << flush;
		trees[i] = ConstructTree(selectedSample, 0, "entropy", selectedFeature);
	}
	return NULL;
}
