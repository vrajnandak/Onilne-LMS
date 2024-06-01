#include"../include/Buffer_utils.h"
#include<stdio.h>

void clear_buffers(char *first, ...)
{
	va_list args;
	char *buffer;

	va_start(args,first);

	buffer=first;
	while(buffer!=NULL)
	{
		memset(buffer,0,strlen(buffer));
		buffer=va_arg(args,char *);
	}
}
