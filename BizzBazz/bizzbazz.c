#include "bizzbazz.h"

ssize_t bb_default_write(char * buf, size_t count );

struct bb_info* bb_create( char* separators_ , ssize_t (*default_write)(char*, size_t) )
{

    struct bb_info* info = (struct bb_info* ) malloc(sizeof(struct bb_info));
    if ( info == 0 ){
        goto bb_create_exit_0;
    }

    if ( default_write == NULL)
        info->write = &bb_default_write;
    else
        info->write = default_write;
    if ( separators_ == NULL)
        memcpy( info->separators ," ,;.\n",6);
    else if( strlen(separators_) <= 256 )
        memcpy( info->separators, separators_ , strlen(separators_) );

    info->temp_string_max_len = 10;
    info->temp_string_len = 0;
    info->last_separator = ' ';
    info->temp_string = 
        (char*) malloc(sizeof(char)* info->temp_string_max_len);
    if ( info->temp_string < 0 ) {
        goto bb_create_exit_1;
    }
    return info;
    bb_create_exit_1:
        free(info);
    bb_create_exit_0:
    return NULL;
} 


void bb_destroy( struct bb_info* info)
{
    bb_flush( info);
    free( info->temp_string );
    free( info );
}

int bb_give_byte( char byte, struct bb_info* info)
{
    if ( info->temp_string_len == info->temp_string_max_len){
        char* temp;
        temp = (char*) 
            realloc( info->temp_string, sizeof(char)*info->temp_string_max_len *2);
        if (temp && (info->temp_string_max_len < INT_MAX/2) ) 
            // if len > 10^9 for int = int 32
        {
             info->temp_string = temp;
             info->temp_string_max_len = info->temp_string_max_len*2;
        }
        else
        {
            goto bb_give_byte_exit_0;
        }
    }
    // if it is number
    if ( byte <= '9' && byte >= '0' )
    {
        info->temp_string[ info->temp_string_len++ ] = byte;
        goto bb_give_byte_exit_ok;
    }
    if ( byte == '-')   // is there '-' before the number
                        // it is possible to add some any...
    {
        if ( info->temp_string_len == 0)
        {
            info->temp_string[ info->temp_string_len++ ] = '-';
            goto bb_give_byte_exit_ok;
        }
        else
        {
            goto bb_give_byte_exit_logic_error;
        }
    }
    // is it one of separators
    if ( strchr( info->separators, byte  ) )
    {
        info->last_separator = byte;
        return bb_flush(info);
    }
    if ( byte == EOF)
    {
        info->last_separator = byte;
        return bb_flush(info);
    }

    goto bb_give_byte_exit_logic_error;

    bb_give_byte_exit_ok:
        return 0;

    bb_give_byte_exit_logic_error:
        info->temp_string_len = 0;
    bb_give_byte_exit_0:
        return -1;
}

int bb_flush( struct bb_info* info)
{
    char separator[1];
    if (info->temp_string_len == 0) goto bb_flush_exit_ok;
    char *temp_string ; 
    int temp_string_len;
    if ( info->temp_string[0] == '-'){
        temp_string = info->temp_string+1;
        temp_string_len = info->temp_string_len - 1;
        if (temp_string_len == 0) return -1;
    } else
    {
        temp_string = info->temp_string;
        temp_string_len = info->temp_string_len;
    }
    int is_dividing_by_3, is_dividing_by_5;
    is_dividing_by_3 = bb_dividing_by_3( temp_string, temp_string_len );
    is_dividing_by_5 = bb_dividing_by_5( temp_string, temp_string_len );
    if ( !is_dividing_by_5 && !is_dividing_by_3 )
    {
        if ( !info->write( info->temp_string, info->temp_string_len) )
            goto bb_flush_exit_ok;
        return -1;
    }
    if ( is_dividing_by_5 )
    {
        if ( !info->write( "bizz", 4) );
        else return -1;
    }
    if ( is_dividing_by_3 )
    {
        if ( !info->write( "bazz" , 4) );
        else return -1;
    }
    bb_flush_exit_ok:
        if ( info->last_separator != EOF )
        {   separator[0] = info->last_separator;
            if (info->write(separator, 1)) return -1;
        }
        info->temp_string_len=0;
        return 0;
}

int bb_dividing_by_3( char * string, int len)
{
    int i, temp=0;    
    for ( i = 0; i < len; i++){
        temp += (int)string[i] -(int) '0';
    }
    if ( temp < 10)
    {
        if ( temp ==  9 || temp == 6 ||temp == 3) return 1;
        else return 0;
    }
    char temp_string[20]; // maximal possible for int64, strlen < 10^16
    sprintf(temp_string, "%d", temp); 
    int temp_len = strlen( temp_string );
    return bb_dividing_by_3( temp_string , temp_len);
}

int bb_dividing_by_5( char * string, int len)
{
    if ( string[len-1] == '5' || string[len-1] == '0' )
        return 1;
    else
        return 0;
}

ssize_t bb_default_write(char * buf, size_t count ){
    ssize_t written = 0, len;
    while ( len = write(1, buf + written, count - written))
    {
        written += len;
    }
    if (written < count)
        return -1;
    return 0;
}

