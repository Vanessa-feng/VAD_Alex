/*******************************************************************
*
* 尝试用ffnn.py生成动态库时写的C语言调用函数，读取wav，用来调用动态库ffnn.so
* 编译  gcc __init__.c -ldl -o main  生成main可执行文件
*
* 2017.3.13
*
*********************************************************************/

#include "stdio.h"
#include <dlfcn.h>
typedef unsigned char  uchar;  
typedef unsigned char  uint8;  
typedef unsigned short uint16;  
typedef unsigned long  uint32;  
typedef char           sint8;  
typedef short          sint16;  
typedef long           sint32;  
typedef float          fp32;  
typedef double         fp64;  


typedef enum BOOLEAN  
{  
TRUE  = 1,  
FALSE = 0  
} boolean;  


//wave文件头  
typedef struct WaveHeader  
{  
uint8 riff[4];             //资源交换文件标志  
uint32 size;               //从下个地址开始到文件结尾的字节数  
uint8 wave_flag[4];        //wave文件标识  
uint8 fmt[4];              //波形格式标识  
uint32 fmt_len;            //过滤字节(一般为00000010H)  
uint16 tag;                //格式种类，值为1时，表示PCM线性编码  
uint16 channels;           //通道数，单声道为1，双声道为2  
uint32 samp_freq;          //采样频率  
uint32 byte_rate;          //数据传输率 (每秒字节＝采样频率×每个样本字节数)  
uint16 block_align;        //块对齐字节数 = channles * bit_samp / 8  
uint16 bit_samp;           //bits per sample (又称量化位数)  
} wave_header_t;  

typedef struct WaveStruct  
{  
FILE *fp;                  //file pointer  
wave_header_t header;      //header  
uint8 data_flag[4];        //数据标识符  
uint32 length;             //采样数据总数  
uint32 *pData;             //data  
} wave_t;  
wave_t wave;  


/* 
* open *.wav file 
*/  
void WaveOpen(char *file, int raw, int mono_stereo)  
{  
    uchar temp = 0;  
    uint8 read_bytes = 0;  
    char *channel_mappings[] = {NULL,"mono","stereo"};  
    uint32 total_time = 0;  
    struct PlayTime        //播放时间  
    {  
        uint8 hour;  
        uint8 minute;  
        uint8 second;  
    } play_time;  
    
    if(NULL == (wave.fp=fopen(file, "rb")))                               /* open file */  
    {  
        printf("file %s open failure!\n", file);      
    }  
      
    /* read heade information */  
    if(4 != fread(wave.header.riff, sizeof(uint8), 4, wave.fp))           /* RIFF chunk */  
    {  
         printf("read riff error!\n");  
         return;  
    }  
    if(1 != fread(&wave.header.size, sizeof(uint32), 1, wave.fp))         /* SIZE : from here to file end */  
    {  
        printf("read size error!\n");  
        return;  
    }  
    if(4 != fread(wave.header.wave_flag, sizeof(uint8), 4, wave.fp))      /* wave file flag */  
    {  
        printf("read wave_flag error!\n");  
        return;  
    }  
    if(4 != fread(wave.header.fmt, sizeof(uint8), 4, wave.fp))             /* fmt chunk */  
    {  
        printf("read fmt error!\n");  
        return;  
    }  
    if(1 != fread(&wave.header.fmt_len, sizeof(uint32), 1, wave.fp))       /* fmt length */  
    {  
        printf("read fmt_len error!\n");  
        return;  
    }  
    if(1 != fread(&wave.header.tag, sizeof(uint16), 1, wave.fp))           /* tag : PCM or not */  
    {  
        printf("read tag error!\n");  
        return;  
    }  
    if(1 != fread(&wave.header.channels, sizeof(uint16), 1, wave.fp))      /* channels */  
    {  
        printf("read channels error!\n");  
        return;  
    }  
    if(1 != fread(&wave.header.samp_freq, sizeof(uint32), 1, wave.fp))      /* samp_freq */  
    {  
        printf("read samp_freq error!\n");  
        return;  
    }  
    if(1 != fread(&wave.header.byte_rate, sizeof(uint32), 1, wave.fp))      /* byte_rate : decode how many bytes per second */  
    {                                                                       /* byte_rate = samp_freq * bit_samp */  
        printf("read byte_rate error!\n");  
        return;  
    }  
    if(1 != fread(&wave.header.block_align, sizeof(uint16), 1, wave.fp))       /* quantize bytes for per samp point */  
    {  
        printf("read byte_samp error!\n");  
        return;  
    }  
    if(1 != fread(&wave.header.bit_samp, sizeof(uint16), 1, wave.fp))        /* quantize bits for per samp point */  
    {                                                                        /* bit_samp = byte_samp * 8 */  
        printf("read bit_samp error!\n");  
        return;  
    }  

    printf("%s %ldHz %dbit, DataLen: %ld, Rate: %ld, Length: %2ld:%2ld:%2ld\n",  
           channel_mappings[wave.header.channels],             //声道  
           wave.header.samp_freq,                              //采样频率  
           wave.header.bit_samp,                               //每个采样点的量化位数  
           wave.length,  
           wave.header.byte_rate,  
           play_time.hour,play_time.minute,play_time.second);  
      
      
    //fclose(wave.fp);                                               /* close wave file */  
}  



/* 
* get wave data  
*/  
//uint32* GetWave(void)  
int main(void)
{       
    WaveOpen("/home/fx/fx/VAD/sound/1.wav", 1, 16000);    
    static uint32 buffer[512] = {0};           
    uint16 n = 512;  
    uint16 p = 0;  
    
    p = fread(buffer, sizeof(uint32), n, wave.fp);     
    if(!p)  
    {           
        return 0;  
    }  
    else  
    {   
	/*调用动态库        
        void *foo = dlopen("/home/fx/fx/VAD/alex-master/alex/components/vad/inform_so/build/ffnn_so.so",RTLD_LAZY);
        dlerror();
        void (*fun)(uint32*)=dlsym(foo,"decide");
        //printf("come here!\n"); 
   
        fun(buffer);             
        /*这步有问题,TypeError: 'TensorVariable' object is not callable
         *可能是动态库ffnn.so调用问题(关于调用做了简单的add.py检测,可以输出结果),也可能是ffnn.so本身生成问题(可能性更大,因为python生成.so文件时只生成ffnn.so,可是ffnn.py调用了很多文件)
         
        dlclose(foo);
        */
        
        return 0;      
    }
}  





