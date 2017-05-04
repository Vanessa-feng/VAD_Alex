/********************************************************************
*
* Copyright (c) 2017, Vanessa Feng.
*
* Read sound from wav file.
*
* 2017.4.25
*
*********************************************************************/
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <vector>

using namespace std;
struct wav_struct
{
        unsigned short channel;             //通道数
        unsigned long frequency;            //采样频率
        unsigned short sample_num_bit;      //一个样本的位数
        unsigned long data_size;            //数据长度
        char *data;                         //音频数据 
};

class ReadWav{
public:
	void readwav(char *filename);
	char* get_buf(){
		return buf_c;
	}
	int get_size(){
		return soundsize;
	}
private:
	int soundsize;
	char* buf_c;
	vector<char> sound;
};
