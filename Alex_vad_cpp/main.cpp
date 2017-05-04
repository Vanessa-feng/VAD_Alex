/********************************************************************
*
* Copyright (c) 2017, Vanessa Feng.
*
* Rewrite from Alex-vad-ffnn which is writen by Python, we want to 
* use it to Andrews equipment which writen by Java. Because of better 
* transferring this feature， we need to rewrite it by C or C++.
*
* 打算作为ffnn.cpp的调用函数，读取wav文件
* 传入data即是str型，ffnn中不用unpack
*
* 2017.3.16
*
*********************************************************************/

#include "ffnn.h"
#include "readwav.h"
#include <iostream>
#include <stdlib.h>

#define WINDOW_SIZE (512)
#define SAMPLE_RATE (16000)
#define FRAME_SHIFT (160)

using namespace std;

int main(int argc, char *argv[]){
	
	if (argc == 1){
		printf("请指定wav文件路径！\n");
		exit(0);
	}
	
	//read wav file.
	ReadWav Rw;
	Rw.readwav(argv[1]);
	char *buf_c = Rw.get_buf();
	int soundsize = Rw.get_size();
	
	//VAD:get valid voice
	bool usehamming = true;
	double preemcoef = 0.97;
	int numchans = 26;
	int ceplifter = 22;
	int numceps = 12;
	int n_last_frames = 10;
	int n_prev_frames = 10;
	int lo_freq = 125;
	int hi_freq = 3800;
	bool mel_banks_only = true;
	FFNNVADGeneral ffnn(SAMPLE_RATE, WINDOW_SIZE, FRAME_SHIFT, usehamming, preemcoef, numchans, ceplifter, numceps, n_last_frames, n_prev_frames, lo_freq, hi_freq, mel_banks_only);
	ffnn.decide(buf_c,soundsize);
	

	/*
	if(is_signal){
		cout << 1 << "\n";
	}else{
		cout << 0 << "\n";
	}
	*/
	return 0;
}

