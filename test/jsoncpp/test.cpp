#include "json/json.h"
#include <iostream>
#include <fstream>

using namespace std;
int main()
{
	Json::Reader reader;
	Json::Value root;
	
	ifstream f;
	f.open("../../vad_model.json", std::ios::binary);
	//reader将Json字符串解析到root，root将包含Json里所有子元素 
	if(!reader.parse(f,root)) 
	{
		printf("There some errors..\n");
		return 0;
	}

	int params_size = root.size();
	if(params_size != 13)
	{
	
		printf("Don't get the right type of params\n");
		printf("Please check the process of write model.json\n");		
		return 0;	
	}
	
	//解析json中的对象
	int n_inputs = root["n_inputs"].asInt();
	//cout << n_inputs << endl;

	
	float weight_l2 = root["weight_l2"].asFloat();
	//cout << weight_l2 << endl;	

	//解析数组对象
	const Json::Value n_hidden = root["n_hidden"];
	for(int i=0; i<n_hidden.size(); i++)
	{
		//cout << n_hidden[i] << endl;
	}
	int input_size = root["input_ml"].size();
	const Json::Value input_ml = root["input_ml"];
	cout << input_ml.size() << endl;
	
	for(int i=0; i<input_size; i++)
	{
		//cout << input_ml[i] << "\t";
	}
	
	const Json::Value params = root["params_l"];
	Json::Value weight_1 = params[0];
	Json::Value weight_2 = params[2];
	Json::Value bias_1 = params[1];
	Json::Value bias_2 = params[3];
	//cout << params << endl;
	for(int i=0; i<weight_1.size(); i++)
	{
		for(int j=0; j<weight_1[0].size(); j++)
		{
			//cout << weight_1[i][j] << "\t";
		}
		//cout << endl;
		//cout << endl;
	} 		
	
	cout << params.size() << endl;
	for(int i=0; i<params.size();i++){
		cout << params[i].size() << endl;
		cout << params[i][0].size() << endl;
	}
	/*
	546
	32
	32
	0
	32
	32
	32
	0
	32
	32
	32
	0
	32
	32
	32
	0
	32
	32
	32
	0
	32
	32
	32
	0
	32
	32
	32
	0
	32
	2
	2
	0
	*/
	
	f.close();
	return 0;
}
