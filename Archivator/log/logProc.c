// The same as log/log.c with only one difference. It uses separate process writer against pthread.
// It is better ( I think ) in case of main program crushing.

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdlib.h>

#include "log.h"

typedef struct
{
    int isStarted ;
    int logDes;
    unsigned flags;
    int logLevel;
    int writeBufDes;
    int readBufDes;
    pthread_t writerThreadId;
    struct timeval startTime;
} LOGMAININFO;

LOGMAININFO logMainInfo = {.isStarted = 0};

void * threadWriter( void* param);

int writerProcId;

int logInit(unsigned logLevel, unsigned flags, const char * filename){
    int des;
    int rc;
    int pipefd[2];// I am using pipe as circular buffer.
    // let us guess that it is can be started in only one thread immediately.
    if ( logMainInfo.isStarted == 1){ 
            goto log_exit_0;
    }
    
    if ( logLevel >= LOG_LEVELS_COUNT || logLevel < 0 ){
        goto log_exit_0;
    }

    if (filename == NULL){
        des = 2; // stderr
    } else{
        des = open(filename, O_APPEND | O_CREAT | O_WRONLY, 0666);
        if ( des == -1 )
	{
            goto log_exit_0;
	}
    }
    rc = pipe( pipefd );
    if ( rc == -1 ){
        goto log_exit_1;
    }

    logMainInfo.flags = flags;
    logMainInfo.readBufDes = pipefd[0] ; 
    logMainInfo.writeBufDes = pipefd[1] ;
    logMainInfo.logLevel = logLevel ; 
    logMainInfo.logDes = des ; 
    logMainInfo.isStarted = 1;
    gettimeofday(&logMainInfo.startTime,0);
    
    rc = fork();
    if ( rc == 0 ){ //child
        close(logMainInfo.writeBufDes);
        threadWriter( NULL);
    }else if ( rc > 0){ // parent
        writerProcId = rc;
        close(logMainInfo.readBufDes);
    }else{
        close( pipefd[1]);
        goto log_exit_1;
    }
    logMesg("log.c", __LINE__ , "LOG", LOG_INFO, "Log started successfully"); 
    return 0;

    log_exit_2:
        close( pipefd[0]);
        close( pipefd[1]);
    log_exit_1:
        close( des);
    log_exit_0:
        return -1;
}   

int logClose(){
    int rc;
    if ( logMainInfo.isStarted == 0){
        return -1;
    }
    logMainInfo.isStarted = 0;
    close( logMainInfo.writeBufDes );
    waitpid( writerProcId , &rc , 0 );
    close( logMainInfo.readBufDes );
    close( logMainInfo.logDes);
    if( logMainInfo.logDes != 2){ //stderr
        close( logMainInfo.logDes );
    }
    return 0;
}   

// thread which is writing data out
void * threadWriter( void* param){
    char buf[WRITER_ATOM_SIZE];
    ssize_t len;
    ssize_t writtenlen;
    while( len = read( logMainInfo.readBufDes, buf, WRITER_ATOM_SIZE )){
        writtenlen = write( logMainInfo.logDes, buf, len );
        if( writtenlen != len );
    }
    close( logMainInfo.readBufDes);
    close( logMainInfo.logDes);
    exit(0);
}

float timedifference_msec(struct timeval t0, struct timeval t1)
{    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f; }


// main logging function
int logMesg( const char *fname, int lineno ,char* group, int priority ,const char* str,...)
{   
    if ( logMainInfo.isStarted !=1 ){// it is ok if after chect it becomes closed immediately
        return -1;
    }
    if ( priority < logMainInfo.logLevel ){
        return 0;
    }
    char buf[MAX_MESG_SIZE];
    int len_preamb = 0;
    int len_mesg; 
    ssize_t len;
    va_list argptr;
    va_start(argptr, str);
    if ( logMainInfo.flags & LOG_PRINT_TIME ){
        struct timeval t1;
        float elapsed;
        gettimeofday(&t1, 0);
        elapsed = timedifference_msec( logMainInfo.startTime, t1);
        len_preamb+=snprintf( buf + len_preamb, MAX_MESG_SIZE-1-len_preamb, "[%0.3f]",  elapsed);
    }
    if ( logMainInfo.flags & LOG_PRINT_LEVEL_DESCRIPTION ){
        len_preamb+=snprintf( buf + len_preamb, MAX_MESG_SIZE-1-len_preamb, 
		"%s:", LOGLEVELS_DESCRIPTIONS[priority]); 
    }
    if ( logMainInfo.flags & LOG_PRINT_GROUP ){
        len_preamb+=snprintf( buf + len_preamb, MAX_MESG_SIZE-1-len_preamb, "%s:", group); 
    }
    if ( logMainInfo.flags & LOG_PRINT_FILE ){
        len_preamb+=snprintf(  buf + len_preamb, MAX_MESG_SIZE-1-len_preamb, "%s:", fname); 
    }
    if ( logMainInfo.flags & LOG_PRINT_LINE ){
        len_preamb+=snprintf(  buf + len_preamb, MAX_MESG_SIZE-1-len_preamb, "%d:",  lineno); 
    }
    len_mesg = vsnprintf( buf+len_preamb, MAX_MESG_SIZE-len_preamb-1 , str ,argptr);
	buf[len_preamb+len_mesg] = '\n';
    len = write( logMainInfo.writeBufDes, buf, len_preamb + len_mesg + 1);
    if ( len != len_preamb + len_mesg + 1 );
    return len+1;
}


