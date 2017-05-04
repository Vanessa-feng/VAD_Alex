/********************************************************************
*
* Copyright (c) 2017, Vanessa Feng.
*
* Read sound from wav file.
*
* 2017.4.25
*
*********************************************************************/

#include "readwav.h"

void ReadWav::readwav(char *filename){

	fstream wavfile;
    	wav_struct WAV;
	wavfile.open(filename, ios::binary | ios::in);
	
    	wavfile.seekg(0x14);
    	wavfile.read((char*)&WAV.channel, sizeof(WAV.channel));
 
    	wavfile.seekg(0x18);
    	wavfile.read((char*)&WAV.frequency, sizeof(WAV.frequency));
 
    	wavfile.seekg(0x22);
    	wavfile.read((char*)&WAV.sample_num_bit, sizeof(WAV.sample_num_bit));
 
    	wavfile.seekg(0x28);
    	wavfile.read((char*)&WAV.data_size, sizeof(WAV.data_size));
 
    	WAV.data = new char[WAV.data_size];

	wavfile.seekg(0x2c);
    	wavfile.read((char *)WAV.data, sizeof(char)*WAV.data_size);
	/*
        cout << "音频通道数  ：" << WAV.channel << endl;
    	cout << "采样频率    ：" << WAV.frequency << endl;
    	cout << "样本位数    ：" << WAV.sample_num_bit << endl;
    	cout << "音频数据大小：" << WAV.data_size << endl;
	*/
	
	for (unsigned long i=0; i<WAV.data_size; i++){
		char data_sound = WAV.data[i];
		sound.push_back(data_sound);
	}
	soundsize = WAV.data_size;
	buf_c = &sound[0];
    	wavfile.close();
}

