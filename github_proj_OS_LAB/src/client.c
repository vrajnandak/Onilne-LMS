#include"../include/client.h"

//Client
int main()
{
	//Establish Connection to server to access OLMS.(Done through simple socket connection, TCP used to ensure secure connection and for data security purposes).
	int fsock;
	struct sockaddr_in server;	

	//Clearing out the server values to default values incase there is a non-default value.
	memset(&server,0,sizeof(server));

	//Creating a half socket which uses TCP connection to establish a connection with the server. Will be used as the full socket only for communicating with the server.
	fsock=socket(AF_INET,SOCK_STREAM,0);
	if(fsock<0)
	{
		perror("client socket(): ");
		exit(1);
	}

	//Assigning values of the server's values.
	server.sin_family=AF_INET;
        server.sin_port=htons(SRV_PORT_NUM);
        server.sin_addr.s_addr=inet_addr(SRV_ADDRESS);

	//Connecting to the server.
        int r=connect(fsock,(struct sockaddr *)&server,sizeof(server));
	if(r<0)
	{
		perror("client connect(): ");
		exit(1);
	}
	printf("connection established\n");

	//Buffers to receive Information from Server.
	char flag[OPTION_SIZE];			//Buffer to receive the flag.
	char server_msg[BUFFER_SIZE];		//Buffer to receive the information based on flag.

	//Now, connection has been safely established with the server.
	//Until user chooses to exit from the application, keep performing different activities sent by the server.
	do
	{
		//Clearing the buffers.
		CLEAR_BUFFERS(flag,server_msg)

		//Always receive a flag. Execute the code corresponding to that flag in the switch statement.
		recv(fsock,flag,OPTION_SIZE,0);

		if(strcmp(flag,CHOOSE_ONE_OPTION_SEND)==0)				//Prints the received msg, takes user input of max size as OPTION_SIZE and sends to server. Used for inputs like homepage_option,client_option.
		{
			recv(fsock,server_msg,BUFFER_SIZE,0);

			char user_input[OPTION_SIZE];
			printf("%s\n",server_msg);
			printf("Choosing option: ");
			scanf("%s",user_input);
			printf("\n");

			send(fsock,user_input,OPTION_SIZE,0);
		}
		else if(strcmp(flag,EXECUTE_COMMAND_LINE)==0)				//Executes the scan_all_inputs() function.
		{
			scan_all_inputs_and_send(fsock);
		}
		else if(strcmp(flag,ONLY_PRINT_MSG)==0)					//Simply prints the message on the client terminal.
		{
			recv(fsock,server_msg,BUFFER_SIZE,0);
			printf("%s\n",server_msg);
		}
		else if(strcmp(flag,EXIT_PROG)==0)					//Exits the program.
		{
			recv(fsock,server_msg,BUFFER_SIZE,0);
			printf("%s\n",server_msg);
			break;
		}
		//sleep(1);
	}while(1);

	return 0;
}

void scan_all_inputs_and_send(int fsock)
{
	char COMMAND_LINE[BUFFER_SIZE];
	recv(fsock,COMMAND_LINE,BUFFER_SIZE,0);

	//Buffer to hold input values.
	char INPUTS[BUFFER_SIZE]={0};

	//Parsing the string using '?' as delimiter.
	char *operation=strtok(COMMAND_LINE,"?");
	while(operation!=NULL)
	{
		if(strcmp(operation,"END SCAN")==0)
		{
			break;
		}
		else if(strcmp(operation,"START SCAN")!=0)
		{
			//Operation is going to be an input operation only.
			printf("%s\n",operation);

			//Getting space separated string as input.
			char input[BUFFER_SIZE];
			if(scanf(" %[^\n]",input)!=1)
			{
				perror("client scan_all_inputs_and_send(), in scanf(): ");
				exit(1);
			}
			getchar();	//Consuming the newline character left in input buffer.

			//Building the final client input to send to server.
			strcat(input, "?");
			strcat(INPUTS, input);

			printf("Current INPUTS: %s\n", INPUTS);
		}

		//Getting the next token.
		operation=strtok(NULL,"?");
	}

	INPUTS[strlen(INPUTS)-1]='\0';

	//Sending the client input to server.
	send(fsock,INPUTS,BUFFER_SIZE,0);
}
