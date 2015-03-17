#define DEBUG
#define LOG_GROUP "LOG_TEST"
#include "log.h"
#include <assert.h> 

int main(){
    assert( logInit(LOG_INFO, LOG_PRINT_FILE | LOG_PRINT_LINE , NULL) !=-1 );
    assert( LOGMESG(LOG_INFO,"%d, %d\n", 5, 74) !=-1 );
    assert( LOGMESG(LOG_ALL,"%d, %d\n", 5, 84) !=-1 );
    assert( LOGMESG(LOG_FATAL,"Valera!!! %d,\n", 5) !=-1 );
    assert( logClose() !=-1 );
    assert( logInit(LOG_INFO, LOG_PRINT_GROUP | LOG_PRINT_LINE |
             LOG_PRINT_TIME | LOG_PRINT_FILE , "log_test_out_file" ) !=-1 );
    assert( LOGMESG(LOG_INFO,"%d, %d\n", 5, 74) !=-1 );
    assert( LOGMESG(LOG_ALL,"%d, %d\n", 5, 84)!=-1 );
    assert( LOGMESG(LOG_FATAL,"Valera!!! %d,\n", 5)!=-1 );
    assert( logClose()!=-1 );
    return 0;
}
