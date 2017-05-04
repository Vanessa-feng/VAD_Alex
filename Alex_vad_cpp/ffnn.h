/********************************************************************
*
* Copyright (c) 2017, Vanessa Feng.
*
* Rewrite from Alex-vad-ffnn which is writen by Python, we want to 
* use it to Andrews equipment which writen by Java. Because of better 
* transferring this feature， we need to rewrite it by C or C++.
*
*********************************************************************/
#ifndef _FFNN_H
#define _FFNN_H

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <deque>
#include <math.h>
#include "FFNN.h"
#include "MFCC.h"

#define uSize (10)
using namespace std;


class FFNNVADGeneral {
    
public:
	FFNNVADGeneral(int sample_rate, int frame_size, int frame_shift, bool usehamming, double preemcoef, int num_chans, int ceplifter, int num_ceps, int n_last_frames, int n_prev_frames, int lofreq, int hifreq, bool mel_banks_only);
	virtual ~FFNNVADGeneral();
	void decide(char* input,int sound_size);
	double logsumexp(double a,double b){
		double sum_num = exp(a)+exp(b);
		return log(sum_num);
	}
	void smooth(int* result, int len);
	
private:
	FILE *model;   
        
	int samplerate;
	unsigned int framesize;
	int frameshift;
	double preemcoef;
	int numchans;
	int filter_length;	
	int ceplifter;
	int numceps;
	int n_frames;
	int lofreq;
	int hifreq;
	bool mel_only;
	 

        static const double last_decision = 0.0;
	vector<double> audio_recorded;
	deque<double> log_probs_speech;
	deque<double> log_probs_sil;
	double log_prob_speech_avg;
	double prob_speech_avg;

	vector<double> mfcc;
	double* prob_result;
	double* signal_in;
	bool is_signal;	
	int* result;
};


#endif 
