 #define _GNU_SOURCE
#include <stdlib.h>     //exit()
#include <signal.h>     //signal()
#include <time.h>
#include "ADS1256.h"
#include "stdio.h"
#include <sys/time.h>
#include <string.h>
#include <math.h>
#include <stdint.h>     /* intmax_t */
#include <inttypes.h>   /* strtoimax, PRIdMAX, SCNdMAX */
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


void  Handler(int signo)
{
    //System Exit
    printf("\r\nEND                  \r\n");
    DEV_ModuleExit();

    exit(0);
}
int main(int argc,char *argv[])
{
    int32_t  ADC[3];
    double ADC_V[3];
    printf("Program Started\r\n");
    
    int ampgain=1; //Board amplifier gain
    double numgain = 1.0; //Numerical gain applied in the program 
    int si=1;//Board sample interval
    char *dpath = "./";//Path for the output
    
    //Start parameter file read
    if(argc!=2) {
        printf("No parameter file passed. Using default parameters.\n");
        printf("\nAmp Gain: %d \nNum Gain: %f \nSample Interv: %d \nOutput path : %s \n\n",ampgain,numgain,si,dpath);}
    else {
        FILE * fr = fopen(argv[1], "rt");
        if(fr == NULL){printf("file %s not found", argv[1]);}
        char tmpstr1[16];
        char tmpstr2[64];
        char tempbuff[100];
        while(!feof(fr)) {
           if (fgets(tempbuff,100,fr)) {
            sscanf(tempbuff, "%15s : %64s", tmpstr1, tmpstr2);
            //printf("<<%s>>\n",  tmpstr1);
            //printf("<<%s>>\n",  tmpstr2);
            if (strcmp(tmpstr1,"AmpGain")==0) {
                 ampgain = atoi(tmpstr2);} 
            else if (strcmp(tmpstr1,"NumGain")==0) {
                 numgain =atof(tmpstr2);}
            else if (strcmp(tmpstr1,"SampInt")==0) {
                 si = atoi(tmpstr2);} 
            else if (strcmp(tmpstr1,"Path")==0) {
                 dpath =tmpstr2;}    
            else{printf("Unrecongized parameter : \"%s\"\n", tmpstr1);}
            }
         }
    fclose(fr);
    printf("\nAmp Gain: %d \nNum Gain: %f \nSample Interv: %d \nOutput path : %s \n\n",ampgain,numgain,si,dpath);
    }
    //end parameter file read
    
    double _conversionFactor = numgain;
    double _pga = 64;
    DEV_ModuleInit();
    DEV_Digital_Write(DEV_CS_PIN, 1);

    // Exception handling:ctrl + c
    signal(SIGINT, Handler);

    if(ADS1256_init() == 1){
        printf("InitFail\r\nEND\r\n");
        DEV_ModuleExit();
        exit(0);
    }
    //---------
    //Parametrizing the sample rate and gain according to parameter file
    ADS1256_GAIN inigain=ADS1256_GAIN_64;
    ADS1256_DRATE inidrate=ADS1256_7500SPS;
    switch (ampgain) 
    { 
       case 64: inigain=ADS1256_GAIN_64; 
                _pga = 64;
               break; 
       case 32: inigain=ADS1256_GAIN_32; 
               _pga = 32;
               break; 
       case 16: inigain=ADS1256_GAIN_16; 
               _pga = 16;
               break; 
       case 8: inigain=ADS1256_GAIN_8; 
               _pga = 8;
               break;
       case 4: inigain=ADS1256_GAIN_4; 
               _pga = 4;
               break; 
       case 2: inigain=ADS1256_GAIN_2; 
               _pga = 2;
               break; 
       case 1: inigain=ADS1256_GAIN_1; 
               _pga = 1;
               break;
       default: inigain=ADS1256_GAIN_64; 
               _pga = 64;
               break;   
    } 
    
    switch (si) 
    { 
       case 30000: inidrate=ADS1256_30000SPS; 
                break; 
       case 15000: inidrate=ADS1256_15000SPS; 
                break;
       case 7500: inidrate=ADS1256_7500SPS; 
               break;
       case 3750: inidrate=ADS1256_3750SPS; 
               break; 
       case 2000: inidrate=ADS1256_2000SPS; 
               break;
       case 1000: inidrate=ADS1256_1000SPS; 
               break;
       case 500: inidrate=ADS1256_500SPS; 
               break;
       case 100: inidrate=ADS1256_100SPS; 
               break;
       case 60: inidrate=ADS1256_60SPS; 
               break; 
       case 50: inidrate=ADS1256_50SPS; 
               break;
       case 30: inidrate=ADS1256_30SPS; 
               break;
       case 25: inidrate=ADS1256_25SPS; 
               break;
       case 15: inidrate=ADS1256_15SPS; 
               break;
       case 10: inidrate=ADS1256_10SPS; 
               break;
       case 5: inidrate=ADS1256_5SPS; 
               break;
       case 2: inidrate=ADS1256_2d5SPS; 
               break;
       default: inidrate=ADS1256_7500SPS; 
                break;   
    } 
    ADS1256_ConfigADC(inigain,inidrate);
    printf("Setting Amplifier gain to: %d and SPS to: %d\n",inigain,inidrate);
    //---------
    DEV_Digital_Write(DEV_CS_PIN, 0);

    FILE *pFile;

    time_t current_time;
    struct tm * time_info;
    char timeString[20];  // space for "HH:MM:SS\0"
    int changed = 0;
    char pfilename[64];  //for full path name

    time(&current_time);
    time_info = localtime(&current_time);
    strftime(timeString, sizeof(timeString), "%Y%m%d_%H%M%S.txt", time_info);
    strcpy(pfilename,dpath);
    strcat(pfilename,timeString);
    //pFile = fopen( timeString,"w" );
    pFile = fopen(pfilename,"w" );
    
    if( NULL == pFile ){
        printf( "open failure" );
        return 1;
    }

    int     fd[2];
    pid_t   childpid;
    char    readbuffer[1024*128];

    pipe(fd);

    if((childpid = fork()) == -1)
    {
            perror("fork");
            exit(1);
    }

    if(childpid == 0)
    {
            /* Child process closes up input side of pipe */
            /* For Reading the data from ADS1256 */
            close(fd[0]);

            struct timespec spec_last;
            clock_gettime(CLOCK_REALTIME, &spec_last);


            struct timespec sleep_spec;
            sleep_spec.tv_nsec = 1000000000 - spec_last.tv_nsec; // start from second
            nanosleep(&sleep_spec, NULL);


            clock_gettime(CLOCK_REALTIME, &spec_last);
            while(1){
                    // Reading three channel according to Figure 19 in ADS1256 Datasheet
                    while(DEV_Digital_Read(DEV_DRDY_PIN) == 1);
                    DEV_SPI_WriteByte(CMD_WREG | REG_MUX);
                    bcm2835_delayMicroseconds(30);
                    DEV_SPI_WriteByte(0x00);
                    DEV_SPI_WriteByte((0 << 4) | 1); // Switch to Diff ch 0
                    DEV_SPI_WriteByte(CMD_SYNC);
                    bcm2835_delayMicroseconds(100);
                    DEV_SPI_WriteByte(CMD_WAKEUP);
                    DEV_SPI_WriteByte(CMD_RDATA);
                    bcm2835_delayMicroseconds(30);
                    ADC[2] = ADS1256_Read_ADC_Data_Lite();
                      if (ADC[2]  & 0x00800000) {
                        ADC[2]  |= 0xff000000;
                      }
                    ADC_V[2] =  (((double)ADC[2] / 0x7FFFFF) * ((2 * 2.5) / (float)_pga)) *  _conversionFactor;

                    while(DEV_Digital_Read(DEV_DRDY_PIN) == 1);
                    DEV_SPI_WriteByte(CMD_WREG | REG_MUX);
                    bcm2835_delayMicroseconds(30);
                    DEV_SPI_WriteByte(0x00);
                    DEV_SPI_WriteByte((2 << 4) | 3); // Switch to Diff ch 1
                    DEV_SPI_WriteByte(CMD_SYNC);
                    bcm2835_delayMicroseconds(100);
                    DEV_SPI_WriteByte(CMD_WAKEUP);
                    DEV_SPI_WriteByte(CMD_RDATA);
                    bcm2835_delayMicroseconds(30);
                    ADC[0] = ADS1256_Read_ADC_Data_Lite();
                    if (ADC[0]  & 0x00800000) {
                        ADC[0]  |= 0xff000000;
                      }
                    ADC_V[0] =  (((double)ADC[0] / 0x7FFFFF) * ((2 * 2.5) / (float)_pga)) *  _conversionFactor;

                    while(DEV_Digital_Read(DEV_DRDY_PIN) == 1);
                    DEV_SPI_WriteByte(CMD_WREG | REG_MUX);
                    bcm2835_delayMicroseconds(30);
                    DEV_SPI_WriteByte(0x00);
                    DEV_SPI_WriteByte((4 << 4) | 5); // Switch to Diff ch 2
                    DEV_SPI_WriteByte(CMD_SYNC);
                    bcm2835_delayMicroseconds(100);
                    DEV_SPI_WriteByte(CMD_WAKEUP);
                    DEV_SPI_WriteByte(CMD_RDATA);
                    bcm2835_delayMicroseconds(30);
                    ADC[1] = ADS1256_Read_ADC_Data_Lite();
                    if (ADC[1]  & 0x00800000) {
                        ADC[1]  |= 0xff000000;
                      }
                    ADC_V[1] =  (((double)ADC[1] / 0x7FFFFF) * ((2 * 2.5) / (float)_pga)) *  _conversionFactor;

                    time_t          s;  // Seconds
                    struct timespec spec;

                    clock_gettime(CLOCK_REALTIME, &spec);

                    s  = spec.tv_sec;
                    long durition;
                    durition = spec.tv_nsec - spec_last.tv_nsec + (spec.tv_sec -  spec_last.tv_sec)*1000000000UL;

                    char cBuffer[200];
                    sprintf(cBuffer, "%"PRIdMAX".%09ld,%.9lf,%.9lf,%.9lf,%09ld\n",(intmax_t)s, spec.tv_nsec,ADC_V[0],ADC_V[1],ADC_V[2],durition);
                    write(fd[1], cBuffer, (strlen(cBuffer)+1));
                    clock_gettime(CLOCK_REALTIME, &spec_last);
            }
            exit(0);
    }
    else
    {
            /* Parent process closes up output side of pipe */
            /* For writing the data to files */
            close(fd[1]);
            fcntl(fd[0],F_SETPIPE_SZ,1048576); 
            while(1){
                read(fd[0], readbuffer, sizeof(readbuffer));
                //printf("Received string: %s", readbuffer);
                fwrite(readbuffer,1,strlen(readbuffer),pFile);
                time(&current_time);
                time_info = localtime(&current_time);
                if ((time_info->tm_min % 5 == 0) & (changed == 0)) //Switch file per 5 min
                {
                    strftime(timeString, sizeof(timeString), "%Y%m%d_%H%M%S.txt", time_info);
                    fclose(pFile);
                    strcpy(pfilename,dpath);
                    strcat(pfilename,timeString);
                    //pFile = fopen( timeString,"w" );
                    pFile = fopen(pfilename,"w" );
                    changed = 1;
                }
                if ((time_info->tm_min % 5 == 1) & (changed == 1))
                {
                    changed = 0;
                }
            }

    }
    
    DEV_Digital_Write(DEV_CS_PIN, 1);
    return 0;
}

