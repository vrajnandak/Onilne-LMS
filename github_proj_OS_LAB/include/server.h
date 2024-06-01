#ifndef SERVER_H
#define SERVER_H

#include"common_utils.h"
#include"Buffer_utils.h"
#include"homepage.h"
#include"Admin_functions.h"
#include"Customer_functions.h"
#include"locking.h"
#include"validate_functions.h"
#include"logging.h"

//FILE MACROS
#define LOG_FILE "Library_logfile.bin"			//File to record all the Transactions happening in the OLMS.
#define OLMS_LIBRARY "Library.bin"			//File to store all the Books in the Library.
#define ADMIN_CRED_FILE "Admins.bin"			//File to store all the Admins registered with the Library.
#define CUSTOMER_CRED_FILE "Customers.bin"		//File to store all the Customers registered with the Library.
#define OLMS_LIBRARY_REQUEST "Library_request.bin"	//File to store all the Book requests by the Customers.

//ACTIVITY MACROS
#define NULL_POINTER_ACCESS "error: Trying to access NULL pointer"
#define VALIDATION_SUCCESS "Validating input: Success. Field values match the data types specified."
#define VALIDATION_FAILURE "Validating input: Failure. Please enter 'r' to retry, else you will exit."
#define DB_UPDATE_SUCCESS "Successfully updated Database."
#define THANK_YOU_FOR_VISITING "Thank You For Visiting The OLMS. We Await Your Return."
#define NOT_REGISTERED_IN_OLMS "No Match Found in OLMS. Please Recheck Credentials. Enter 'r' to retry, else you will exit."
#define CLIENT_ALREADY_REGISTERED "You Are Already Registered in the OLMS. Enter 'r' if you would like to signup again, else you will exit."

//SOCKET RELATED MACROS
#define ONLY_SEND(CONN_SOCK,BUFFER)	send(CONN_SOCK,ONLY_PRINT_MSG,OPTION_SIZE,0);\
					send(CONN_SOCK,BUFFER,BUFFER_SIZE,0);
#define QUIT_PROG(CONN_SOCK,BUFFER)	send(conn_sock,EXIT_PROG,OPTION_SIZE,0);\
					send(conn_sock,BUFFER,BUFFER_SIZE,0);
#define RECEIVE_ONE_OPTION(CONN_SOCK,BUFFER_SEND,BUFFER_RECV)	send(CONN_SOCK,CHOOSE_ONE_OPTION_SEND,OPTION_SIZE,0);\
								send(CONN_SOCK,BUFFER_SEND,BUFFER_SIZE,0);\
								recv(CONN_SOCK,BUFFER_RECV,OPTION_SIZE,0);
#define EXECUTE_COMMANDS(CONN_SOCK,BUFFER_SEND,BUFFER_RECV)	send(CONN_SOCK,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);\
								send(CONN_SOCK,BUFFER_SEND,BUFFER_SIZE,0);\
								recv(CONN_SOCK,BUFFER_RECV,BUFFER_SIZE,0);

//OTHER MACROS
#define MAX_PENDING_CONNECTIONS 10			//Max backlog for creating socket to listen for the server.
#define IS_FILE_CREATED_CHECK(FD) if(FD<0){if(errno!=EEXIST){perror("Error creating file open(): "); exit(1);}}
#define IF_ERROR_THEN_EXIT_CHECK_PTR(PTR,PERROR_MSG,SEND_MSG,CONN_SOCK)	if(PTR==NULL){perror(PERROR_MSG);QUIT_PROG(CONN_SOCK,SEND_MSG);exit(1);}
#define FSEEK_CHK(RET_VAL,CONN_SOCK) if(RET_VAL!=0){perror("ERROR fseek(): ");QUIT_PROG(CONN_SOCK,"FSEEK ERROR");exit(1);}

void choose_cred_file(char *buff,char *option);		//Function to choose between the 2 CRED files.
long long int is_entity_in_file(admin *cred_adm,admin *read_adm,customer *cred_cust,customer *read_cust,int entity_type,FILE *ptr,struct flock *lock,int conn_sock);	//Function to check if the Entity is in the file.
void choose_num_parse(char *main_option,char *inner_option,int *num_parse);	//Function to choose number of times to parse based on login,signup options.
void update_cred_file(admin *curr_adm,customer *curr_cust,FILE *ptr,struct flock *lock,int rec_size,long long int offset,int conn_sock);

#endif
