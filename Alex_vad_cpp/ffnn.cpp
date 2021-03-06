/********************************************************************
*
* Copyright (c) 2017, Vanessa Feng.
*
* Rewrite from Alex-vad-ffnn which is writen by Python, we want to 
* use it to Andrews equipment which writen by Java. Because of better 
* transferring this feature， we need to rewrite it by C or C++.
*
*********************************************************************/
#include "ffnn.h"

FFNNVADGeneral::FFNNVADGeneral(int sample_rate, int frame_size, int frame_shift, bool usehamming, double preem_coef, int num_chans, int ceplifter, int num_ceps, int n_last_frames, int n_prev_frames, int lo_freq, int hi_freq, bool mel_banks_only){
	
	samplerate = sample_rate;
	framesize = frame_size;
	frameshift = frame_shift;
	preemcoef = preem_coef;	
	filter_length = 2;	
	numchans = num_chans;	
	numceps = num_ceps;
	lofreq = lo_freq;
	hifreq = hi_freq;	
	n_frames = n_last_frames+n_prev_frames;
	mel_only = mel_banks_only;

	signal_in = new double[framesize];
	for(int i=0;i<framesize;i++){
		signal_in[i]=0.0;
	}
}

FFNNVADGeneral::~FFNNVADGeneral()
{
	delete[] signal_in;
}

	
void FFNNVADGeneral::decide(char* input,int sound_size)
{
	short* __restrict signal = (short *)input;
	int num_frame = 0;
	audio_recorded.reserve(framesize);
	/*Error:只能添加signal[0]，用指针不能使每项/32767.0
	short *data = &signal[0];
	audio_recorded_in.push_back(*data); */
	
	MFCCFrontEnd front_end(samplerate, framesize, preemcoef, numchans, numceps, n_frames, lofreq, hifreq, mel_only);

	front_end.getMFCCs();
	
	FFNNetwork ffnn;
	ffnn.load("./vad_model.json");
	//cout << "Now input the vadio to ffnn.." << endl;
	//cout << "-----------------------------------------------------" << endl;
	//deque<double> add_frame;
	//double prob_speech_add;
	
	int FrameLen = sound_size/(2*frameshift);
	result = new int[FrameLen];
	for(int i=0 ;i<FrameLen; i++){
		result[i] = 0;
	}

	while(FrameLen - num_frame){
		for(int i=0; i<framesize; i++){
			signal_in[i] = double(signal[i]);
			//signal_in[i] = double(signal[i])/32767.0;
			audio_recorded.push_back(signal_in[i]);
			//cout << signal_in[i] << endl;
		
		}
		/*for(int i=0; i<framesize; i++){
			cout << audio_recorded[i] << endl;
		}
		*/
		
		mfcc = front_end.param(audio_recorded, mfcc); 

		/********************* FFNN  **************************/
		/*prob_result = [prob_sil,prob_speech]
		*/

		if(mfcc.empty()){
			cout << "Error: Input null to ffnn\n";
			exit(0);
		}
		prob_result = ffnn.predict_normalise(&mfcc[0]);
		//cout << prob_result[0]<<"\t"<<prob_result[1] <<endl;
		double prob_sil = *prob_result;
		double prob_speech = *(prob_result+1);
	
		log_probs_speech.push_back(log(prob_speech));
		log_probs_sil.push_back(log(prob_sil));
		while (log_probs_speech.size()>2)
		{
			log_probs_speech.pop_front();
		}
		while (log_probs_sil.size()>2)
		{
			log_probs_sil.pop_front();
		}	
		/********************* prob result **************************/
		log_prob_speech_avg = 0.0;
		for(int i=0;i<log_probs_speech.size();i++)
		{
			log_prob_speech_avg += log_probs_speech[i] - logsumexp(log_probs_speech[i],log_probs_sil[i]);
		}
	
		log_prob_speech_avg /= log_probs_speech.size();

		prob_speech_avg = exp(log_prob_speech_avg);
		/*
		add_frame.push_back(prob_speech_avg);
		prob_speech_add += prob_speech_avg;
		if(add_frame.size() > 15){
			prob_speech_add -= add_frame[0];
			add_frame.pop_front();
		}
		*/
		
		//cout << "frame: " << num_frame << "\ttime: " << num_frame*(frameshift/(float)samplerate) << "\tlast_decision: " << prob_speech_avg << endl;
		//cout << (num_frame-7)*(frameshift/(float)samplerate) << "\t" << prob_speech_add/15.0 << endl;
		
		is_signal = (prob_speech_avg >0.5 ? 1:0);
		if(is_signal){
			result[num_frame] = 1;
		}

		mfcc.clear();
	 	signal+=frameshift; //语音数据指针向后移frameshift
		num_frame++;
		audio_recorded.clear();
	}
	smooth(result, FrameLen);
}

	/************************** result smooth **************************/

void FFNNVADGeneral::smooth(int* result, int len){
	// 判断语音区域较短(10帧),认为噪音
	int* d = new int[len-1];
	for(int i=0; i<len-1; i++)
	{
		d[i] = 0;
	} 
	vector<int> vadStart, vadEnd;
	for(int i=1; i<len; i++)
	{			
		d[i] = result[i]-result[i-1];
		if(d[i] == 1){
			vadStart.push_back(i);
		}
		else if(d[i] == -1){
			vadEnd.push_back(i);
		}
	}
	//delete[] d;

	int temp;
	vector<int> VAD_start, VAD_end;
	for(int i=0; i<vadStart.size(); i++)
	{
		temp = vadEnd[i] - vadStart[i];
		if(temp >= uSize){
			VAD_start.push_back(vadStart[i]);
			VAD_end.push_back(vadEnd[i]);
		}
	}

	// 判定语音前后增加语音段(10帧),认为语音
	// 相邻语音段间隔较小(10帧),认为语音
	int interval;
	vector<int> sound_start, sound_end;
	sound_start.push_back(VAD_start[0]);
	sound_end.push_back(VAD_end[0]);
	for(int i=0; i<VAD_start.size(); i++)
	{
		VAD_start[i] -= uSize;
		VAD_end[i] += uSize;
		if(i==0){
			continue;
		}
		interval = VAD_start[i] - VAD_end[i-1];
		if(interval > uSize){
			sound_start.push_back(VAD_start[i]);	
			sound_end.push_back(VAD_end[i]);
		}else{
			sound_end.pop_back();
			sound_end.push_back(VAD_end[i]);
		}
	}
	for(int i=0; i<sound_start.size(); i++)	
	{
		cout << sound_start[i] << "\t" << sound_end[i] << endl;	
	}
}



