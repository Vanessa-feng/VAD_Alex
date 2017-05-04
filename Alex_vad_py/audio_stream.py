#!usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np
from pyaudio import PyAudio,paInt16
from datetime import datetime
import wave
from Tkinter import *
import sys
from ffnn import FFNNVADGeneral

import logging
# import chardet    # 查看编码

# define of params
NUM_SAMPLES =160
FRAMERATE = 16000
CHANNELS = 1
SAMPWIDTH = 2
FORMAT = paInt16
TIME = 125
FRAMESHIFT = 160

def save_wave_file(filename,data):
    '''save the date to the wav file'''
    wf = wave.open(filename,'wb')
    wf.setnchannels(CHANNELS)
    wf.setsampwidth(SAMPWIDTH)
    wf.setframerate(FRAMERATE)
    wf.writeframes("".join(data))
    wf.close()

def my_button(root,label_text,button_text,button_stop,button_func,stop_func):
    '''create label and button'''
    label = Label(root,text=label_text,width=30,height=3).pack()
    button = Button(root,text=button_text,command=button_func,anchor='center',width=30,height=3).pack()
    button = Button(root,text=button_stop,command=stop_func,anchor='center',width=30,height=3).pack()


def record_wave():
    '''open the input of wave'''
    pa = PyAudio()
    stream = pa.open(format=FORMAT,
                     channels=CHANNELS,
                     rate=FRAMERATE,
                     input=True,
                     frames_per_buffer=NUM_SAMPLES)   #一个buffer存NUM_SAMPLES个字节,作为一帧
    vad = FFNNVADGeneral('/home/fx/fx/VAD/alex-master/alex/tools/vad_train/model_voip/vad_nnt_546_hu32_hl1_hla6_pf10_nf10_acf_1.0_mfr20000_mfl20000_mfps0_ts0_usec00_usedelta0_useacc0_mbo1_bs100.tffnn', 
    			filter_length=2, sample_rate=16000, framesize=512, frameshift=160,
    			usehamming=True, preemcoef=0.97, numchans=26, ceplifter=22, numceps=12, 
    			enormalise=True, zmeansource=True, usepower=True, usec0=False, 
    			usecmn=False, usedelta=False, useacc=False, n_last_frames=10, 
    			n_prev_frames=10, lofreq=125, hifreq=3800, mel_banks_only=True)

    save_buffer = []
    count = 0
    logging.basicConfig(level=logging.INFO,
                        filename='log.txt',
                        filemode ='w',
                        format='%(message)s')
    while count < TIME*4:
        string_audio_data = stream.read(NUM_SAMPLES)	
        result = vad.decide(string_audio_data)
        frame = count*NUM_SAMPLES/float(FRAMESHIFT)
	time = count*NUM_SAMPLES/float(FRAMERATE)
	logging.info('frame: '+str(frame)+' time: '+str(time)+' prob: '+str(result))

        save_buffer.append(string_audio_data)
        count += 1
        #chardet.detect(string_audio_data)
        print "."
   
    filename = datetime.now().strftime("%Y-%m-%d_%H_%M_%S")+".wav"
    save_wave_file(filename,save_buffer)
      
    save_buffer = []
    print "filename,saved."

def record_stop():
    # stop record the wave
    sys.exit(0)

def main():
    root = Tk()
    root.geometry('300x200+200+200')
    root.title('record wave')
    my_button(root,"Record a wave","clik to record","stop recording",record_wave,record_stop)
    root.mainloop()
    
    

if __name__ == "__main__":
    main()


