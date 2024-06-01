#include"../include/common_utils.h"
void parse_the_creds(char *ind_creds[8],char *buff_recv,int num_parse)
{
	char tmp_buffer[BUFFER_SIZE];
	strcpy(tmp_buffer,buff_recv);

	int i=0;
	char *token=strtok(tmp_buffer,"?");
	if(token==NULL)
	{
		perror("strtok() in parse_the_creds\n");
	}
	ind_creds[i]=token;
	i++;

	for(;i<num_parse;i++)
	{
		token=strtok(NULL,"?");
		if(token==NULL)
		{
			perror("strtok() in parse_the_creds\n");
		}
		ind_creds[i]=token;
	}
}
