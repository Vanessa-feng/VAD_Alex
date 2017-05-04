/********************************************************************
*
* Copyright (c) 2017, Vanessa Feng.
*
* Rewrite from Alex-vad-ffnn which is writen by Python, we want to 
* use it to Andrews equipment which writen by Java. Because of better 
* transferring this feature， we need to rewrite it by C or C++.
*
*********************************************************************/

#include "FFNN.h"
using namespace std;

FFNNetwork::FFNNetwork()
{	
	//cout << "Initialize parameters .." << endl;
	n_inputs = 0;
	n_outputs = 0;
	
	n_hidden_units = 0;
	n_hidden_layers = 0;
	
	prev_frames = 0;
	next_frames = 0;
	amplify_center_frame = 0;

	batch_size = 0;
	weight_l2 = 1e-6;
	hidden_activation = "tanh";	

	amp_min = 1.0/amplify_center_frame;
	amp_max = 1.0;
	

}

FFNNetwork::~FFNNetwork(){
	delete[] amp_prev;
	delete[] amp_next;

	delete[] amp;
	delete[] amp_vec;
	delete[] input_m;
	delete[] input_std;
	for(int i=0; i<layer_num; i++){
		delete [] weight[i];
		delete [] bias[i];
	}
	delete [] weight;
	delete [] bias;
	delete[] prob_result;
}

/** @function
********************************************************************************
<PRE>
* 函数名: get_array()
* 功能: 给数组分配内存并初始化
* 主调函数: load()
</PRE>
*******************************************************************************/
void FFNNetwork::get_array()
{
	amp_prev = new double[prev_frames];
	for(int i=0; i<prev_frames; i++){
		amp_prev[i] = amp_min + (double(i)/prev_frames)*(amp_max-amp_min);
	}
	amp_next = new double[next_frames];
	for(int i=0; i<next_frames; i++){
		amp_next[i] = amp_min + (double(i)/next_frames)*(amp_max-amp_min);
	}
	amp = new double[add_frames];
	for(int i=0; i<prev_frames; i++){
		amp[i] = amp_prev[i];
	}
	amp[prev_frames] = amp_max;
	int j = 0;
	for(int i=prev_frames+1; i<add_frames; i++){
		j+=2;
		amp[i] = amp_next[i-j];
	}
	amp_vec = new double[n_inputs];
	for(int i=0; i<num_chans; i++){
		for(int j=0; j<add_frames; j++){
			amp_vec[i*add_frames+j] = amp[j];
		}
	}
		
	input_m = new double[n_inputs];
	for(int i=0; i<n_inputs; i++){
		input_m[i] = 0.0;
	}
	input_std = new double[n_inputs];
	for(int i=0; i<n_inputs; i++){
		input_std[i] = 0.0;
	}
	prob_result = new double[2];

}

/** @function
********************************************************************************
<PRE>
* 函数名: softmax()
* 功能: 计算一组数据的softmax值
* 用法: 用于神经网络最后输出层的分类器
* 参数:
* [IN] vec: 神经网络最后隐藏层输出的向量
* [OUT] vec: softmax计算后得到的向量
* 主调函数: build_model()
</PRE>
*******************************************************************************/
VectorXd FFNNetwork::softmax(VectorXd vec)
{
	double sum = 0.0;
	for(int i=0; i<vec.size(); i++)
	{
		vec(i) = exp(vec(i));
		sum += vec(i);
	}
	for(int i=0; i<vec.size(); i++)
		vec(i) /=sum;
	return vec;
}


/** @function
********************************************************************************
<PRE>
* 函数名: build_model()
* 功能: 神经网络过程
* 参数:
* [IN] input: 输入层输入的特征向量(这里是MFCC特征)
* [OUT] y: 输出层输出分类特征(这里是二维分类)
* 调用: softmax()
* 主调函数: predict_normalise()
</PRE>
*******************************************************************************/
VectorXd FFNNetwork::build_model(double *input){
	VectorXd x = VectorXd::Zero(n_hidden[0]);
	VectorXd y = VectorXd::Zero(n_hidden[1]);
	for(unsigned int i=0; i<n_hidden[0]; i++)
		{
			x(i) = *(input+i);
		}
	//cout << x << "\t";
	
	for(int k=0; k<layer_num; k++)
	{	
		/*	
		for(int i=0; i<n_hidden[k+1]; i++){
			cout << bias[k][i] << "\t";
		}
		cout << endl;
		cout << endl;
		*/
		//数组转Eigen矩阵
		Map<MatrixXd,0> w(weight[k], n_hidden[k+1], n_hidden[k]);
		Map<VectorXd,0> b(bias[k],n_hidden[k+1]);

		y = w * x + b;
		
		if (k!=layer_num-1)
		{	
			x = VectorXd::Zero(n_hidden[k+1]);
			for (unsigned int i=0; i<n_hidden[k+1]; i++){
				x(i) = tanh(y(i));	
			}
			y = VectorXd::Zero(n_hidden[k+2]);
		}else{
			y = softmax(y);

		}	
	}
	return y;
}

/** @function
********************************************************************************
<PRE>
* 函数名: load()
* 功能: 导入训练模型参数
* 参数:
* [IN] file_name: 训练得到的模型参数文件.json
* 主调函数: ffnn::param()
</PRE>
*******************************************************************************/
int FFNNetwork::load(const char* file_name){
	//cout << "-----------------------------------------------------" << endl;
	//cout << "Load the model params..." << endl;
	Json::Reader reader;
	Json::Value root;
	
	ifstream f;
	f.open(file_name, std::ios::binary);
	if(!reader.parse(f,root))
	{
		cout << "There some errors at load->JsonError!" << endl;
		return -1;
	}
	
	int file_size = root.size();
	if(file_size != 13)
	{
	
		cout << "Don't get the right type of params" << endl;
		cout << "Please check the process of write model.json" << endl;		
		return -1;	
	}
	//cout << "Open the modelfile.." << endl;
	//解析json中的对象
	
	n_inputs = root["n_inputs"].asInt();
	n_outputs = root["n_outputs"].asInt();
	weight_l2 = root["weight_l2"].asDouble(); 
	prev_frames = root["prev_frames"].asInt();
	next_frames = root["next_frames"].asInt();
	batch_size = root["batch_size"].asInt();

	//给数组分配内存
	//cout << "Allocate memory to the array.." << endl;
	add_frames = prev_frames + 1 + next_frames;
	num_chans = n_inputs/add_frames;
	get_array();
	
	/*
	cout << "n_inputs is: " << n_inputs << endl;	
	cout << "n_outputs is: " << n_outputs << endl;
	cout << "prev_frames is: " << prev_frames << endl;		
	cout << "next_frames is: " << next_frames << endl;
	cout << "batch_size is: " << batch_size << endl;
	cout << "num_chans is : " << num_chans << endl;		
	*/

	Json::Value input_ml = root["input_ml"];
	for(unsigned int i=0; i<input_ml.size(); i++)
	{
		input_m[i] = input_ml[i].asDouble();
	}
		
	const Json::Value input_stdl = root["input_std"];
	for(unsigned int i=0; i<input_stdl.size(); i++)
	{
		input_std[i] = input_stdl[i].asDouble();
	}

	const Json::Value ampl = root["amp"];
	for(unsigned int i=0; i<ampl.size(); i++)
	{
		amp[i] = ampl[i].asDouble();
	}
	
	const Json::Value amp_vecl = root["amp_vec"];
	for(unsigned int i=0; i<amp_vecl.size(); i++)
	{
		amp_vec[i] = amp_vecl[i].asDouble();
	}
	
	
	//考虑add_hidden_layers
	//cout << "Consider the add_hidden_layers.." << endl;
	n_hidden.push_back(n_inputs);
	const Json::Value n_hiddenl = root["n_hidden"];
	for(int i=0; i<n_hiddenl.size(); i++)
	{
		n_hidden.push_back(n_hiddenl[i].asInt());
	}
	n_hidden.push_back(n_outputs);
	layer_num = n_hidden.size()-1;

	//cout << "Get the trained params of weights and bias.." << endl;
	const Json::Value params_l = root["params_l"];
	
	//cout << "The shape of weights and bias: " << endl;

	weight = new double*[layer_num];
	bias = new double*[layer_num];
	for(unsigned int k=0; k<layer_num; k++)
	{		
		weight[k] = new double[n_hidden[k]*n_hidden[k+1]];
		bias[k] = new double[n_hidden[k+1]];
		
		for(unsigned int i=0; i<n_hidden[k]; i++)
		{
			
			for(unsigned int j=0; j<n_hidden[k+1]; j++)
			{
			
				weight[k][i*n_hidden[k+1]+j] = params_l[k*2][i][j].asDouble();
				//cout << i <<"\t"<< j << "\t" <<weight[0][i*params_l[k*2].size()+j] << endl;
			}
			
		}

		for(unsigned int i=0; i<n_hidden[k+1]; i++)
		{
			bias[k][i] = params_l[k*2+1][i].asDouble();
		}
	}
	/*
	cout << "Layer " << k << endl;
	cout << "------------------------" << endl;	
	cout << "weights: " << "[" << layer_num << "," << n_hidden[k] << "x" << n_hidden[k+1] << "]" << endl;
	
	cout << "bias: " << "[" << 2 << "," << n_hidden[k+1] << "]" << endl;
	cout << endl;	
	}
	
	cout << "The shape of input_m: " << input_ml.size() << endl;
	cout << "The shape of input_std: " << input_stdl.size() << endl;
	cout << "The shape of amp: " << ampl.size() << endl;
	cout << "The shape of amp_vec: " << amp_vecl.size() << endl;
	cout << "The shape of input_std: " << input_stdl.size() << endl;
	
	cout << "Finished load params!~" << endl;
	cout << "-----------------------------------------------------" << endl;
	cout << endl;
	*/
	
	f.close();
	return 0;
}


/** @function
********************************************************************************
<PRE>
* 函数名: predict_normalise()
* 功能: MFCC特征归一化，并进行神经网络检测过程
* 参数:
* [IN] input: MFCC特征向量
* [OUT] prob_result: 神经网络输出层(这里分别是speech和sil的分类特征--prob) 
* 调用: build_model()
* 主调函数: ffnn::param()
</PRE>
*******************************************************************************/
double* FFNNetwork::predict_normalise(double *input){
	for(unsigned int i=0; i<n_inputs; i++)
	{
		*(input+i) -= input_m[i];
		*(input+i) /= input_std[i];
		*(input+i) *= amp_vec[i];
		//cout << *(input+i) << "\t";
	}
	y = build_model(input);
	for(int i=0; i<2; i++)
	{
		prob_result[i] = *(y.data()+i);
	}
	return prob_result;
}


//*************************************************************























