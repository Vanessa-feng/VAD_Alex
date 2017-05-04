#!/usr/bin/env python
# coding: utf-8

#import sys
#sys.path.append("/home/fx/fx/VAD/alex-master")

import wave
import numpy as np
from ffnn import FFNNVADGeneral

def main():

	f = wave.open(r"/home/fx/fx/VAD/sound/1.wav","rb")  
       
	soundParams = f.getparams()  
	nchannels, sampwidth, framerate, nframes = soundParams[:4]  

	str_data  = f.readframes(nframes)  
	f.close()  

	# wave_data = np.fromstring(str_data,dtype = np.short)  

	vad = FFNNVADGeneral('/home/fx/fx/VAD/alex-master/alex/tools/vad_train/model_voip/vad_nnt_546_hu32_hl1_hla6_pf10_nf10_acf_1.0_mfr20000_mfl20000_mfps0_ts0_usec00_usedelta0_useacc0_mbo1_bs100.tffnn', 
			filter_length=2, sample_rate=16000, framesize=512, frameshift=160,	usehamming=True, preemcoef=0.97, numchans=26, ceplifter=22, numceps=12, 
			enormalise=True, zmeansource=True, usepower=True, usec0=False, 
			usecmn=False, usedelta=False, useacc=False, n_last_frames=10, 
			n_prev_frames=10, lofreq=125, hifreq=3800, mel_banks_only=True)

	result = vad.decide(str_data)
	
if __name__ == "__main__":
	main()
        
