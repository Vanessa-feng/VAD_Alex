/********************************************************************
*
* Copyright (c) 2017, Vanessa Feng.
*
* Rewrite from Alex-vad-ffnn which is writen by Python, we want to 
* use it to Andrews equipment which writen by Java. Because of better 
* transferring this feature， we need to rewrite it by C or C++.
*
*********************************************************************/
#include "MFCC.h"

MFCCFrontEnd::MFCCFrontEnd(int sample_rate, int frame_size, double preemcoef, int numchans, int num_ceps, int n_frames, int lofreq, int hifreq, bool mel_banks_only){
	
	samplerate = sample_rate;
	framesize = frame_size;
	FFTsize = frame_size;
	filt_size = FFTsize/2+1;
	FiltNum = numchans;
	numceps = num_ceps;
	HIGH = hifreq;
	LOW = lofreq;
	Hamming = new double[framesize];
	
	//energy = 0.0;
	preemcoef_m = preemcoef;
	mel_only = mel_banks_only;
	n_frames_m = n_frames;

	data.reserve(framesize);
	mel_energy = new double[FiltNum];
	for(int i=0; i<FiltNum; i++){
		mel_energy[i] = 0.0;
	}

	FiltWeight = new double*[FiltNum];
	for(int i=0; i<FiltNum; i++){
		FiltWeight[i] = new double[filt_size];
		for(int j=0; j<filt_size; j++){
			FiltWeight[i][j] = 0.0;
		}
	}
	w = new double*[FiltNum]; 
	for(int i=0; i<FiltNum; i++){
		w[i] = new double[filt_size];
		for(int j=0; j<filt_size; j++){
			w[i][j] = 0.0;
		}
	}

	mfcc_queue = new double[FiltNum*(1+n_frames)];
	for(int i=0; i<FiltNum*(1+n_frames); i++){
		mfcc_queue[i] = 0.0;
	}
}

MFCCFrontEnd::~MFCCFrontEnd(){
	delete [] Hamming;
	delete [] mel_energy;
	for(int i=0; i<FiltNum; i++){
		delete [] FiltWeight[i];
		delete [] w[i];
	}
	delete [] FiltWeight;
	delete [] w;
}



/********************************** getFMCCs **************************************/
void MFCCFrontEnd::getMFCCs(){
	prior = 0.0;
	LOGENERGY = 0;
	if (HIGH>(int) (samplerate/2))    
	   	HIGH=(int) (samplerate/2);
	if (LOW>HIGH)             
	   	LOW=(int)(HIGH/2);
	InitHamming();                    //Create a Hamming window of length framesize
	InitFilt(FiltWeight, FiltNum);    // Initialize filter weights to all zero
	CreateFilt(FiltWeight, FiltNum, samplerate, HIGH, LOW);    	
					  // Compute filter weights:FiltWeight[i][j]
	/*print FiltWeight	
 	for(int i=0;i<FiltNum;i++){
		for(int j=0;j<filt_size;j++){
			cout <<FiltWeight[i][j]<<"\t";
		}
		cout << endl;
	}*/
}

vector<double> MFCCFrontEnd::param(vector<double> buffer, vector<double> mfcc){		
	out_frame.reserve(framesize);
	preemphasis(buffer, out_frame);	
				
        HammingWindow(out_frame, data); 	
	//energy=FrmEnergy(buffer);       //Get frame energy without windowing
	
	computerFFT(data, zero_padded); 	
	mag_square(zero_padded, fft_mag); // This step does magnitude square for the first half of FFT
        Mel_EN(FiltWeight,FiltNum, fft_mag, mel_energy); // This step computes output log energy of each channel
	
	if (mel_only){
		Ceps_melonly(mel_energy, mfcc_queue);
	}
	else{
		Cepstrum(mel_energy, mfcc_queue);
		/**** consider energy ****/
		//if (LOGENERGY)          // whether to include log energy term or not
		//mfcc_queue.push_back(energy);
	}
	for(int i=0; i<FiltNum*(n_frames_m+1); i++)
	{
		mfcc.push_back(mfcc_queue[i]);
	}

	data.clear();
	out_frame.clear();
	zero_padded.clear();              // clear up fft vector
	fft_mag.clear();                  // clear up fft magnitude 
	//index++;
	return mfcc;
}



/*************************************预处理************************************/
/*预加重：高通滤波器
 *@param vector frame:data of one frame
 *@return vector out_frame.
 */
void MFCCFrontEnd::preemphasis(vector<double> frame, vector<double> &out_frame){
	
	double sum_frame = 0.0;
	double mean_frame = 0.0;	
	for(int i=0; i<framesize; i++){
		sum_frame += frame[i];
	}	
	mean_frame = sum_frame/framesize;
	for(int i=0; i<framesize; i++){
		frame[i] -= mean_frame;
	}

	out_frame.push_back(frame[0] - preemcoef_m * prior);
	double temp;
	for(int i=1; i<framesize; i++)
	{
		temp = frame[i] - preemcoef_m * frame[i-1];
		out_frame.push_back(temp);
	}
	prior = frame[framesize-1];
	//cout << prior << endl;
}

void MFCCFrontEnd::InitHamming(){
	double two_pi=8.0F*atan(1.0F);
	
	double temp;
	for(int i=0; i<framesize; i++)
	{
		Hamming[i] = (double)(0.54-0.46*cos((double) i*two_pi/(double)(framesize-1)));
	}
}

/*加窗：海明窗
 *@param buf data
 */
void MFCCFrontEnd::HammingWindow(vector<double> buf, vector<double> &data){
	for(int i=0; i<framesize; i++)
	{
		data.push_back(buf[i]*Hamming[i]);
	}
}
//回看输入---每帧数据




/********************************** FFT ****************************************/
/*傅立叶变换
 *@param:fftlen=FFTsize--ComputerFFT() operate FFT(FFTsize,vec)
 *引用输出vector，操作FFT为了得到vec
 */
void MFCCFrontEnd::FFT(const unsigned long & fftlen, vector<complex<double> >& vec) 
{ 		 
	unsigned long ulPower = 0;  
	unsigned long fftlen1 = fftlen - 1; 
	while(fftlen1 > 0) 
	{ 
		ulPower++; 
		fftlen1=fftlen1/2; 
	} 

	bitset<sizeof(unsigned long) * 8> bsIndex;
	unsigned long ulIndex; 
	unsigned long ulK; 
	for(unsigned long p = 0; p < fftlen; p++) 
	{ 
		ulIndex = 0; 
		ulK = 1; 
		bsIndex = bitset<sizeof(unsigned long) * 8>(p); 
		for(unsigned long j = 0; j < ulPower; j++) 
		{ 
			ulIndex += bsIndex.test(ulPower - j - 1) ? ulK : 0; 
			ulK *= 2; 
		} 

		if(ulIndex > p) 
		{ 
			complex<double> c = vec[p]; 
			vec[p] = vec[ulIndex]; 
			vec[ulIndex] = c; 
		} 
	} 

	vector<complex<double> > vecW; 
	for(unsigned long i = 0; i < fftlen / 2; i++) 
	{ 
		vecW.push_back(complex<double>(cos(2 * i * PI / fftlen) , -1 * sin(2 * i * PI / fftlen))); 
	} 

	unsigned long ulGroupLength = 1; 
	unsigned long ulHalfLength = 0;  
	unsigned long ulGroupCount = 0;  
	complex<double> cw; 
	complex<double> c1;  
	complex<double> c2;  
	for(unsigned long b = 0; b < ulPower; b++) 
	{ 
	ulHalfLength = ulGroupLength; 
	ulGroupLength *= 2; 
	for(unsigned long j = 0; j < fftlen; j += ulGroupLength) 
	{ 
		for(unsigned long k = 0; k < ulHalfLength; k++) 
		{ 
			cw = vecW[k * fftlen / ulGroupLength] * vec[j + k + ulHalfLength]; 
			c1 = vec[j + k] + cw; 
			c2 = vec[j + k] - cw; 
			vec[j + k] = c1; 
			vec[j + k + ulHalfLength] = c2; 
			} 
		} 
	} 
} 


void MFCCFrontEnd::computerFFT(vector<double> data,vector<complex<double> >& vec) // This function does zero padding and FFT
{	
	for(int i=0;i<FFTsize;i++)     // This step does zero padding
	{
		if(i<framesize)
		{	
			complex<double> temp(data[i],0.0);
			vec.push_back(temp);
		}
		else
		{
			complex<double> temp(0.0,0.0);
			vec.push_back(temp);
		}
	}
	FFT(FFTsize, vec);    // Compute FFT
}


/********************************* FILTER ***************************************/
/*初始化滤波器，权重为0
 *@param num_filt是语音数据的framesize么
 *
 */
void MFCCFrontEnd::InitFilt(double **w, int num_filt)
{
	int i,j;
	for (i=0;i<num_filt;i++)
		for (j=0;j<filt_size;j++)
			*(*(w+i)+j)=0.0;
}


void MFCCFrontEnd::CreateFilt(double **w, int num_filt, int samplerate, int high, int low)
{
	int df = samplerate/FFTsize;    // FFT interval
	int indexlow=round((double) FFTsize*(double) low/(double) samplerate); // FFT index of low freq limit
	int indexhigh=round((double) FFTsize*(double) high/(double) samplerate)+1; // FFT index of high freq limit
	//cout << indexlow <<"\t" << indexhigh<<endl;
	double melmax=2595.0*log10(1.0+(double) high/700.0); // mel high frequency
	//2097.06	
	double melmin=2595.0*log10(1.0+(double) low/700.0);  // mel low frequency
	//185.169
	double melinc=(melmax-melmin)/(double) (num_filt+1); //mel half bandwidth
	//70.8107
	double melcenters[num_filt];        // mel center frequencies
	double fcenters[num_filt];          // Hertz center frequencies
	int indexcenter[num_filt];         // FFT index for Hertz centers
	int indexstart[num_filt];   	   //FFT index for the first sample of each filter
	int indexstop[num_filt];    	   //FFT index for the last sample of each filter
	double increment,decrement; 	   // increment and decrement of the left and right ramp
	//double sum=0.0;
	int i,j;
	for (i=1;i<=num_filt;i++)
	{
		melcenters[i-1]=(double) i*melinc+melmin;   // compute mel center frequencies
		fcenters[i-1]=700*(pow(10.0,melcenters[i-1]/2595)-1.0); // compute Hertz center frequencies
		indexcenter[i-1]=round(fcenters[i-1]/df); // compute fft index for Hertz centers	
		//cout << indexcenter[i-1] << endl;	 
   	}
	for (i=1;i<=num_filt-1;i++)  // Compute the start and end FFT index of each channel
	{
		indexstart[i]=indexcenter[i-1];
		indexstop[i-1]=indexcenter[i];		
	}
	indexstart[0]=indexlow;
	indexstop[num_filt-1]=indexhigh;
	for (i=1;i<=num_filt;i++)
	{
		increment=1.0/((double) indexcenter[i-1]-(double) indexstart[i-1]); // left ramp
		for (j=indexstart[i-1];j<=indexcenter[i-1];j++)
			w[i-1][j]=((double)j-(double)indexstart[i-1])*increment;

		decrement=1.0/((double) indexstop[i-1]-(double) indexcenter[i-1]); // right ramp
	  	for (j=indexcenter[i-1];j<=indexstop[i-1];j++)
			w[i-1][j]=1.0-((double)j-(double)indexcenter[i-1])*decrement;		 
   	}
	/*
   	for (i=1;i<=num_filt;i++)     // Normalize filter weights by sum
   	{
       	for (j=1;j<=filt_size;j++)
	      	sum=sum+w[i-1][j-1];
	for (j=1;j<=filt_size;j++)
	{
	      	//w[i-1][j-1]=w[i-1][j-1]/sum;
		cout << w[i-1][j-1] << "\t";
	}	
	sum=0.0;
	cout << endl;
	}*/
}


/********************************** Cepstrum ************************************/

void MFCCFrontEnd::mag_square(vector<complex<double> > &vec, vector<double> &vec_mag) // This function computes magnitude squared FFT
{
	int i;
	double temp;
	for (i=1;i<=filt_size;i++)
	{
		temp = vec[i-1].real()*vec[i-1].real()+vec[i-1].imag()*vec[i-1].imag();
		vec_mag.push_back(temp);
	}
}

void MFCCFrontEnd::Mel_EN(double **w,int num_filt, vector<double>& vec_mag, double * M_energy) // computes log energy of each channel
{
	int i,j;
	for (i=1;i<=num_filt;i++)    // set initial energy value to 0
		M_energy[i-1]=0.0F;
   
   	for (i=1;i<=num_filt;i++)
   	{
     		for (j=1;j<=filt_size;j++){
       		  	M_energy[i-1]=M_energy[i-1]+w[i-1][j-1]*vec_mag[j-1];
			//cout << w[i-1][j-1] << "\t";
		}	
     		M_energy[i-1]=(double)(log(M_energy[i-1]));	 
   	}
}


void MFCCFrontEnd::Cepstrum(double *M_energy,double *mfcc_queue)
{
	int i,j;
	double Cep[numceps];
    	for (i=1;i<=numceps;i++)
	{ 
		Cep[i-1]=0.0F;    // initialize to 0
	  	for (j=1;j<=FiltNum;j++)
          		Cep[i-1]=Cep[i-1]+M_energy[j-1]*cos(PI*((double) i)/((double) FiltNum)*((double) j-0.5F)); // DCT transform
      		Cep[i-1]=sqrt(2.0/double (FiltNum))*Cep[i-1];
		mfcc_queue[i-1] = Cep[i-1];   // store cepstrum in this vector
    	}	
	  
}

void MFCCFrontEnd::Ceps_melonly(double *M_energy, double *mfcc_queue)
{	
	
	int t = FiltNum*(1+n_frames_m);
	for (int i=t;i>=FiltNum+1;i--){
		mfcc_queue[i-1] = mfcc_queue[i-1-FiltNum];
	}
	for (int i=1; i<=FiltNum; i++){
		mfcc_queue[i-1] = M_energy[i-1];
	}
	/*
	for(int i=0; i<t; i++){
		cout << mfcc_queue[i] <<"\t";
	}
	*/
	
}
		

















