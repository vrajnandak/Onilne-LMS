#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include<time.h>
#include<ctype.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include"Buffer_utils.h"

//Macros used by multiple files.
#define POINTER_IS_NULL 12
#define NULL_POINTER_ACCESS_ERROR 13
#define IS_POINTER_NULL(X) if(X==NULL){return POINTER_IS_NULL;}
#define MALLOC_FAIL_CHECK(PTR,FUNCTION_NAME,STRUCT_TYPE) if(PTR==NULL){printf("malloc() failed in %s for the struct type: %s\n",FUNCTION_NAME,STRUCT_TYPE);}

//Macros for Buffer sizes.
#define BUFFER_SIZE 1024
#define OPTION_SIZE 2

#define SRV_PORT_NUM 9998

//Macros for Client Execution based on Flags.
#define CHOOSE_ONE_OPTION_SEND "1"
#define EXECUTE_COMMAND_LINE "2"
#define ONLY_PRINT_MSG "3"
#define EXIT_PROG "4"

//Macro to clear out the Buffers using function from 'Buffer_utils.h'.
#define CLEAR_BUFFERS(...) clear_buffers(__VA_ARGS__, NULL);

//Function to apply strtok(), 'num_parse' number of times.
void parse_the_creds(char *ind_creds[8],char *buff_recv,int num_parse);

#endif
