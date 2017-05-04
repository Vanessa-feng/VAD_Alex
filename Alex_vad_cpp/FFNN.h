/********************************************************************
*
* Copyright (c) 2017, Vanessa Feng.
*
* Rewrite from Alex-vad-ffnn which is writen by Python, we want to 
* use it to Andrews equipment which writen by Java. Because of better 
* transferring this feature， we need to rewrite it by C or C++.
*
*********************************************************************/
#ifndef _FFNNETWORK_H
#define _FFNNETWORK_H

#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <Eigen/Dense>
#include "json/json.h"

using namespace std;
using namespace Eigen;
class FFNNetwork{
public:
	FFNNetwork();
	~FFNNetwork();
	void get_array();
	VectorXd softmax(VectorXd vec);
	VectorXd build_model(double *input);
	int load(const char* file_name);
	double* predict_normalise(double *input);
	

private:
	double* input;	

	int n_inputs;
	int n_outputs;
	int n_hidden_units; 
	int n_hidden_layers; 	
	int prev_frames; 
	int next_frames; 
	int add_frames;
	int num_chans;
	int layer_num;
	double amplify_center_frame; 
	int batch_size; 
	const char* hidden_activation; 
	double weight_l2;
	
	
	double amp_min;
	double amp_max;
	double* amp_prev;
	double* amp_next;
	double* amp;
	double* amp_vec;
	double* input_m;
	double* input_std;
	
	double** weight;
	double** bias;
	double* prob_result;

	vector<int> n_hidden;
	VectorXd x;
	VectorXd y;
	MatrixXd w0;
	VectorXd b0;
	MatrixXd w1;
	VectorXd b1;
	
};

#endif


