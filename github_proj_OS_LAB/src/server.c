#include"../include/server.h"

//Concurrent Server.
int main()
{
	//Creating all the Files necessary to maintain the OLMS.
	char *DB_files[5]={OLMS_LIBRARY,OLMS_LIBRARY_REQUEST,ADMIN_CRED_FILE,CUSTOMER_CRED_FILE,LOG_FILE};
	for(int i=0;i<5;i++)
	{
		int fd=open(DB_files[i],O_CREAT | O_EXCL, 0644);
		IS_FILE_CREATED_CHECK(fd)
		close(fd);
	}

	int l_sock;			//Listen socket. Used only to listen to connections.
	int conn_sock;			//Connection socket. Used by accpet() which returns a full socket. Used to communicate with client.
	struct sockaddr_in local;	//Used only for creation of the half socket which listens to connection requests.
	struct sockaddr_in remote;	//Used only for creation of the full socket used to communicate to the client by one of the child processes created by server.
	int remote_size;

	//Creating a half socket.
	l_sock=socket(AF_INET,SOCK_STREAM,0);
	if(l_sock<0)
	{
		perror("server socket(): ");
		exit(1);
	}

	//Assigning values of the possible client's values.
	memset(&local,0,sizeof(local));
	local.sin_family=AF_INET;
	local.sin_addr.s_addr=INADDR_ANY;
	local.sin_port=htons(SRV_PORT_NUM);

	//Associating IP addresses to the socket.
	int r=bind(l_sock,(struct sockaddr *)&local,sizeof(local));
	if(r<0)
	{
		perror("server bind(): ");
		exit(1);
	}

	//Making the socket ready to be able to accept connections.
	r=listen(l_sock,MAX_PENDING_CONNECTIONS);
	if(r<0)
	{
		perror("server listen(): ");
		exit(1);
	}

	//Concurrently keeps creating child processes for each pending connection. Only child communicates with client.
	while(1)
	{
		//Clearing the remote struct before using for creation of full socket to communicate with clients.
		memset(&remote,0,sizeof(remote));

		//Creating the full socket by accepting a pending request.
		remote_size=sizeof(remote);
		conn_sock=accept(l_sock,(struct sockaddr *)&remote,&remote_size);
		if(conn_sock<0)
		{
			perror("server accpet(): ");
			exit(1);
		}
		printf("connection accpeted by server\n");

		//Lock for concurrency control.
		struct flock lock;
		memset(&lock,0,sizeof(lock));

		//Making the child process communicate with the client.
		int r=fork();
		if(r<0)
		{
			perror("server fork(): ");
			exit(1);
		}
		else if(r!=0)	//Parent process
		{
			close(conn_sock);
		}
		else		//Child process
		{
			close(l_sock);			//Closing the socket used for listening.

			//Variables to keep track of the state of the Client in the OLMS.
			int in_home_page=1;
			customer *curr_cust=(customer*)malloc(sizeof(customer));
			admin *curr_adm=(admin*)malloc(sizeof(admin));
			long long int cust_offset=-1;		//The position of the curr_cust record in the CUSTOMER_CRED_FILE.
			long long int adm_offset=-1;		//The position of the curr_adm record in the ADMIN_CRED_FILE.
			int need_to_retry=0;			//Variable to check if there is an error in homepage activity and user needs to retry.
			char retry_recv[OPTION_SIZE];		//String to hold Client option to retry.
			char retry_msg[BUFFER_SIZE];		//String to hold the associated message with the retry.

			//Buffer Variables.
			char buff_send[BUFFER_SIZE];			//Buffer to send messages to the Client.
			char buff_recv[BUFFER_SIZE];			//Buffer to recv messages from the Client.
			char option_recv[OPTION_SIZE];			//Buffer to recv a single option from the Client.

			//Communicating with the Client.
			while(1)
			{
				FILE *log_ptr=fopen(LOG_FILE,"rb+");
				IF_ERROR_THEN_EXIT_CHECK_PTR(log_ptr,"fopen() of Log file: ","Error opening file",conn_sock)
				setvbuf(log_ptr,NULL,_IONBF,0);

				printf("in_home_page: %d\n",in_home_page);
				if(in_home_page)			//In the homepage.
				{
					//Setting the variables to empty.
					curr_cust=NULL;
					curr_adm=NULL;
					cust_offset=-1;
					adm_offset=-1;
					need_to_retry=0;
					CLEAR_BUFFERS(buff_send,buff_recv,option_recv,retry_msg)

					//Additional buffers to copy the state chosen.
					char main_option[OPTION_SIZE];
					char inner_option[OPTION_SIZE];

					//Receiving the selected homepage option from the Client.
					strcpy(buff_send,HOMEPAGE_OPTIONS);
					RECEIVE_ONE_OPTION(conn_sock,buff_send,option_recv)
					strcpy(main_option,option_recv);

					//Receiving the Further options of the homepage options from the Client.
					CLEAR_BUFFERS(buff_send,option_recv)
					get_page_options(buff_send,main_option);
					RECEIVE_ONE_OPTION(conn_sock,buff_send,option_recv)
					strcpy(inner_option,option_recv);

					//Receiving the Required Credentials from the Client based on the above chosen options.
					CLEAR_BUFFERS(buff_send,option_recv,buff_recv)
					get_required_credentials(buff_send,main_option,inner_option);
					EXECUTE_COMMANDS(conn_sock,buff_send,buff_recv)

					//Store the parsed creds into a string buffer.
					char *ind_creds[8];
					int num_parse;
					choose_num_parse(main_option,inner_option,&num_parse);
					parse_the_creds(ind_creds,buff_recv,num_parse);

					//Validating the Credentials received to check if the confirm to the specified datatypes.
					char validate_calls[VALIDATE_STRING_LENGTH];
					choose_validate_calls(validate_calls,main_option,inner_option);
					int validate_result=validate_creds(buff_recv,validate_calls);
					if(validate_result==VALID_CREDENTIALS)
					{
						CLEAR_BUFFERS(buff_send,option_recv)
						ONLY_SEND(conn_sock,VALIDATION_SUCCESS)

						//Choosing the CREDENTIAL filename to operate on.
						char filename[40];
						choose_cred_file(filename,inner_option);

						//Declaring temporary variables to hold the credential values from the Client.
						admin *cred_adm=(admin*)malloc(sizeof(admin));
						customer *cred_cust=(customer*)malloc(sizeof(customer));
						MALLOC_FAIL_CHECK(cred_adm,"main()","admin")
						MALLOC_FAIL_CHECK(cred_cust,"main()","customer")

						//Declaring temporary variables to read an admin or a customer from the file.
						admin *read_adm=(admin*)malloc(sizeof(admin));
						customer *read_cust=(customer*)malloc(sizeof(customer));
						MALLOC_FAIL_CHECK(read_adm,"main()","admin")
						MALLOC_FAIL_CHECK(read_cust,"main()","customer")

						//Variables related to filling the admin/customer struct.
						int entity_type=(strcmp(inner_option,"1")==0)?1:2;
						fill_cred_adm_cust(cred_adm,cred_cust,ind_creds,entity_type,num_parse);

						//Opening the file to operate on.
						char *file_mode=(strcmp(main_option,"1")==0)?"rb":"rb+";
						FILE *cred_ptr=fopen(filename,file_mode);
						IF_ERROR_THEN_EXIT_CHECK_PTR(cred_ptr,"fopen() of cred file: ","Error opening file",conn_sock)

						setvbuf(cred_ptr,NULL,_IONBF,0);

						//Setting the File to the beginning.
						int mv_lp_start=fseek(cred_ptr,0,SEEK_SET);
						FSEEK_CHK(mv_lp_start,conn_sock)

						//Checking to see if the Admin or Customer is registered in the OLMS.
						long long int offset=is_entity_in_file(cred_adm,read_adm,cred_cust,read_cust,entity_type,cred_ptr,&lock,conn_sock);
						if(strcmp(main_option,"1")==0)		//Login.
						{

							if(offset!=-1)	//Login success.
							{
								//Loggin in by updating the curr_cust and curr_adm variables.
								if(entity_type==1)
								{
									adm_offset=offset;
									curr_adm=read_adm;
								}
								else if(entity_type==2)
								{
									cust_offset=offset;
									curr_cust=read_cust;
								}

								printf("going to add log now\n");
								add_log(cred_adm->name,entity_type,"Login SUCCESS",NULL,offset,log_ptr,&lock,conn_sock);
								printf("login log added successfully.\n");
								in_home_page=0;
								continue;
							}
							else		//Admin/Customer hasn't registered with the OLMS. So need to retry to login.
							{
								need_to_retry=1;
								strcpy(retry_msg,NOT_REGISTERED_IN_OLMS);
								add_log(cred_adm->name,entity_type,"Login Error",NULL,offset,log_ptr,&lock,conn_sock);
							}
						}
						else if(strcmp(main_option,"2")==0)	//Signup.
						{
							if(offset==-1)	//Perform signup by writing to file.
							{
								int mv_lp_end=fseek(cred_ptr,0,SEEK_END);
								FSEEK_CHK(mv_lp_end,conn_sock)

								//Writing to the File to perform signup.
								int write_stat;
								acquire_write_lock(cred_ptr,&lock,conn_sock);
								if(entity_type==1)
								{
									write_stat=fwrite(cred_adm,sizeof(admin),1,cred_ptr);
								}
								else if(entity_type==2)
								{
									write_stat=fwrite(cred_cust,sizeof(customer),1,cred_ptr);
								}
								release_acquired_write_lock(cred_ptr,&lock,conn_sock);
								if(write_stat!=1)
								{
									perror("fwrite(): ");
									QUIT_PROG(conn_sock,"fwrite error")
									exit(1);
								}

								in_home_page=1;
								add_log(cred_adm->name,entity_type,"Signup SUCCESS",NULL,offset,log_ptr,&lock,conn_sock);
								printf("log created.\n");
								continue;
							}
							else		//Client already registered.
							{
								need_to_retry=1;
								strcpy(retry_msg,CLIENT_ALREADY_REGISTERED);
								add_log(cred_adm->name,entity_type,"Signup ERROR",NULL,offset,log_ptr,&lock,conn_sock);
							}
						}
					}
					else
					{
						need_to_retry=1;
						strcpy(retry_msg,VALIDATION_FAILURE);
						add_log(ind_creds[0],-1,"Input Validating ERROR",NULL,validate_result,log_ptr,&lock,conn_sock);
					}

					//Retrying the homepage if neccessary.
					if(need_to_retry==1)
					{
						CLEAR_BUFFERS(buff_send,buff_recv,option_recv,retry_recv)

						//Receiving Client's choice to retry.
						RECEIVE_ONE_OPTION(conn_sock,retry_msg,retry_recv)
						if(need_to_retry==1)
						{
							in_home_page=1;
							need_to_retry=0;
							add_log(NULL,-1,"RETRYING",NULL,need_to_retry,log_ptr,&lock,conn_sock);
							continue;
						}
						else	//Quitting.
						{
							strcpy(buff_send,THANK_YOU_FOR_VISITING);
							QUIT_PROG(conn_sock,buff_send)
							need_to_retry=0;
							add_log(NULL,-1,"QUITTING",NULL,need_to_retry,log_ptr,&lock,conn_sock);
							break;
						}
					}
					else
					{
						in_home_page=0;
					}
				}
				else					//In the Functionalities page of Customer/Admin.
				{
					CLEAR_BUFFERS(buff_send,buff_recv,option_recv)
					printf("cleared out all the buffers\n");
					printf("buff_send: %s\n",buff_send);
					printf("buff_recv: %s\n",buff_recv);
					printf("option_recv: %s\n",option_recv);

					//Opening the Files to operate on.
					FILE *lib_ptr=fopen(OLMS_LIBRARY,"rb+");
					FILE *lib_req_ptr=fopen(OLMS_LIBRARY_REQUEST,"rb+");
					FILE *cust_ptr=fopen(CUSTOMER_CRED_FILE,"rb+");
					FILE *adm_ptr=fopen(ADMIN_CRED_FILE,"rb+");
					IF_ERROR_THEN_EXIT_CHECK_PTR(lib_ptr,"fopen() of OLMS_LIBRARY: ","ERROR fopen",conn_sock)
					IF_ERROR_THEN_EXIT_CHECK_PTR(lib_req_ptr,"fopen() of OLMS_LIBRARY_REQUEST: ","ERROR fopen",conn_sock)
					IF_ERROR_THEN_EXIT_CHECK_PTR(cust_ptr,"fopen() of CUSTOMER_CRED_FILE: ","ERROR fopen",conn_sock)
					IF_ERROR_THEN_EXIT_CHECK_PTR(adm_ptr,"fopen() of ADMIN_CRED_FILE: ","ERROR fopen",conn_sock)
					setvbuf(lib_ptr,NULL,_IONBF,0);
					setvbuf(lib_req_ptr,NULL,_IONBF,0);
					setvbuf(cust_ptr,NULL,_IONBF,0);
					setvbuf(adm_ptr,NULL,_IONBF,0);

					if(curr_cust!=NULL)
					{
						printf("customer not null\n");
						strcpy(buff_send,CUSTOMER_FUNCTIONS);
						RECEIVE_ONE_OPTION(conn_sock,buff_send,option_recv)
						printf("option recvd: %s\n",option_recv);

						char function_chosen[OPTION_SIZE];
						strcpy(function_chosen,option_recv);
						printf("function chosen: %s\n",function_chosen);

						CLEAR_BUFFERS(option_recv)

						if(strcmp(function_chosen,"1")==0)		//View library.
						{
							view_library(lib_ptr,conn_sock,&lock);
							add_log(curr_cust->name,2,"Viewed the Books in Library",NULL,0,log_ptr,&lock,conn_sock);
						}
						else if(strcmp(function_chosen,"2")==0)		//Borrow book.
						{
							int ret_val=borrow_book(lib_ptr,conn_sock,curr_cust,&lock,log_ptr);
							if(ret_val==CANT_BORROW_ANOTHER_BOOK)
							{
								ONLY_SEND(conn_sock,"You cannot borrow more books. You have reached the limit")
							}
							else if(ret_val==BOOK_CANT_BE_BORROWED)
							{
								ONLY_SEND(conn_sock,"There are no available copies of the Book to borrow. Please try again next time.")
							}
							else if(ret_val==BOOK_NOT_FOUND)
							{
								ONLY_SEND(conn_sock,"There is no such Book in the Library. Please check the details again.")
							}
							else if(ret_val==SYSTEM_ERROR)
							{
								ONLY_SEND(conn_sock,"There was an error borrowing the Book. Please try again.")
							}
							else if(ret_val==BOOK_BORROW_SUCCESS)
							{
								ONLY_SEND(conn_sock,"The Book was successfully borrowed. Your profile has been Updated.")
							}
							else if(ret_val==NULL_POINTER_ACCESS_ERROR)
							{
								ONLY_SEND(conn_sock,"Error Accessing Pointers.Please try again.")
							}
							else if(ret_val==POINTER_IS_NULL)
							{
								ONLY_SEND(conn_sock,"Pointer is NULL.Please try again.")
							}
							else
							{
								printf("ret_val: %d\n",ret_val);
								ONLY_SEND(conn_sock,"ERROR")
							}
						}
						else if(strcmp(function_chosen,"3")==0)		//Return book.
						{
							int ret_val=return_book(lib_ptr,conn_sock,curr_cust,&lock,log_ptr);
							if(ret_val==NO_BOOK_TO_RETURN)
							{
								ONLY_SEND(conn_sock,"You Currently have no Borrowed books and hence cannot Return a Book.")
							}
							else if(ret_val==SYSTEM_ERROR)
							{
								ONLY_SEND(conn_sock,"There was an error returning the Book. Please try again.")
							}
							else if(ret_val==BOOK_RETURN_SUCCESS)
							{
								ONLY_SEND(conn_sock,"The Book was successfully returned. Your profile has been Updated.")
							}
							else if(ret_val==BOOK_RETURN_ERROR)
							{
								ONLY_SEND(conn_sock,"The title You have entered does not match with the Book in the Library. Please Check the Title again.")
							}
							else if(ret_val==BOOK_NOT_FOUND)
							{
								ONLY_SEND(conn_sock,"There is no such Book in the Library. Please check the details again.")
							}
							else if(ret_val==NULL_POINTER_ACCESS_ERROR)
							{
								ONLY_SEND(conn_sock,"Error Accessing Pointers.Please try again.")
							}
							else if(ret_val==POINTER_IS_NULL)
							{
								ONLY_SEND(conn_sock,"Pointer is NULL.Please try again.")
							}
							else
							{
								printf("ret_val: %d\n",ret_val);
								ONLY_SEND(conn_sock,"ERROR")
							}
						}
						else if(strcmp(function_chosen,"4")==0)		//Request book to be added.
						{
							int ret_val=request_add_book(lib_req_ptr,conn_sock,&lock,log_ptr);
							if(ret_val==SYSTEM_ERROR)
							{
								ONLY_SEND(conn_sock,"There was an error Requesting the Book. Please try again.")
							}
							else if(ret_val==BOOK_REQUEST_SUCCESS)
							{
								ONLY_SEND(conn_sock,"The Request was successfully placed. The Book will be Added once an Admin adds the Book to the Library.")
							}
							else if(ret_val==NULL_POINTER_ACCESS_ERROR)
							{
								ONLY_SEND(conn_sock,"Error Accessing Pointers.Please try again.")
							}
							else if(ret_val==POINTER_IS_NULL)
							{
								ONLY_SEND(conn_sock,"Pointer is NULL.Please try again.")
							}
							else
							{
								ONLY_SEND(conn_sock,"ERROR")
							}
							continue;
						}
						else if(strcmp(function_chosen,"5")==0)		//View profile.
						{
							view_profile_cust(conn_sock,curr_cust);
							add_log(curr_cust->name,2,"Returning Book to Library",NULL,0,log_ptr,&lock,conn_sock);
							continue;
						}
						else if(strcmp(function_chosen,"6")==0)		//Search for a book.
						{
							int ret_val=search_book(lib_ptr,conn_sock,&lock,log_ptr);
							if(ret_val==NULL_POINTER_ACCESS_ERROR)
							{
								ONLY_SEND(conn_sock,"Error Accessing Pointers.Please try again.")
							}
							else if(ret_val==POINTER_IS_NULL)
							{
								ONLY_SEND(conn_sock,"Pointer is NULL.Please try again.")
							}
							else if(ret_val==SYSTEM_ERROR)
							{
								ONLY_SEND(conn_sock,"Error searching for the Book. Please try again")
							}
							else if(ret_val==SEARCH_SUCCESS)
							{
								ONLY_SEND(conn_sock,"Searching Successfull.")
							}
							else
							{
								ONLY_SEND(conn_sock,"ERROR")
							}
							continue;
						}
						else if(strcmp(function_chosen,"7")==0)		//Exit.
						{
							add_log(curr_cust->name,2,"Exiting OLMS",NULL,0,log_ptr,&lock,conn_sock);
							QUIT_PROG(conn_sock,THANK_YOU_FOR_VISITING)
							return 0;
						}

						//Updating the Customer struct into the File.
						update_cred_file(NULL,curr_cust,cust_ptr,&lock,sizeof(customer),cust_offset,conn_sock);
						add_log(curr_cust->name,2,"Updating Information",NULL,0,log_ptr,&lock,conn_sock);
					}
					else if(curr_adm!=NULL)
					{
						strcpy(buff_send,ADMIN_FUNCTIONS);
						RECEIVE_ONE_OPTION(conn_sock,buff_send,option_recv)

						char function_chosen[OPTION_SIZE];
						strcpy(function_chosen,option_recv);

						if(strcmp(function_chosen,"1")==0)		//Add book
						{
							int ret_val=Add_new_book_to_library(lib_req_ptr,lib_ptr,conn_sock,&lock);
							if(ret_val==SYSTEM_ERROR)
							{
								ONLY_SEND(conn_sock,"There was an error Adding the Book to the Library.")
							}
							else if(ret_val==ADD_SUCCESS)
							{
								ONLY_SEND(conn_sock,"Successfully Added a Book to the Library.")
							}
							else if(ret_val==BOOK_NOT_FOUND)
							{
								ONLY_SEND(conn_sock,"No Requests for Books to be Added to the Library.")
							}
							else
							{
								ONLY_SEND(conn_sock,"ERROR")
							}
						}
						else if(strcmp(function_chosen,"2")==0)		//Delete book.
						{
							int ret_val=Delete_book_from_library(lib_ptr,conn_sock,&lock,cust_ptr);
							if(ret_val==SYSTEM_ERROR)
							{
								ONLY_SEND(conn_sock,"There was an error Deleting the Book from the Library.")
							}
							else if(ret_val==DELETE_CANT_BE_DONE)
							{
								ONLY_SEND(conn_sock,"Book cannot be Deleted from the Library. A customer currently has a copy of the Book.")
							}
							else if(ret_val==DELETE_SUCCESS)
							{
								ONLY_SEND(conn_sock,"The Book was successfully Deleted from the Library.")
							}
							else if(ret_val==NO_BOOKS_IN_LIBRARY)
							{
								ONLY_SEND(conn_sock,"There are no Books in the Library. Please Add Books to the Library.")
							}
							else if(ret_val==BOOK_NOT_FOUND)
							{
								ONLY_SEND(conn_sock,"The Book is not present in the Library. Please Check the details again.")
							}
							else
							{
								ONLY_SEND(conn_sock,"ERROR")
							}
						}
						else if(strcmp(function_chosen,"3")==0)		//Update book.
						{
							int ret_val=Update_book_from_library(lib_ptr,cust_ptr,conn_sock,&lock);
							if(ret_val==SYSTEM_ERROR)
							{
								ONLY_SEND(conn_sock,"There was an error Updating the Book.")
							}
							else if(ret_val==UPDATE_CANT_BE_DONE)
							{
								ONLY_SEND(conn_sock,"The Book cannot be Updated as there is a Customer currently with a copy of the Book.")
							}
							else if(ret_val==UPDATE_SUCCESS)
							{
								ONLY_SEND(conn_sock,"The Book details were successfully updated.")
							}
							else if(ret_val==NO_BOOKS_IN_LIBRARY)
							{
								ONLY_SEND(conn_sock,"There are no books in the library. Please Add Books to the Library.")
							}
							else if(ret_val==BOOK_NOT_FOUND)
							{
								ONLY_SEND(conn_sock,"Please check the Details of the Book. There is no such Book in the Library.")
							}
							else
							{
								ONLY_SEND(conn_sock,"ERROR")
							}
						}
						else if(strcmp(function_chosen,"4")==0)		//View all Customers
						{
							Print_all_customers(cust_ptr,conn_sock,&lock);
						}
						else if(strcmp(function_chosen,"5")==0)		//View profile.
						{
							View_profile_adm(conn_sock,curr_adm);
						}
						else if(strcmp(function_chosen,"6")==0)		//View Library.
						{
							View_library(lib_ptr,conn_sock,&lock);
						}
						else if(strcmp(function_chosen,"7")==0)		//Search for a book.
						{
							int ret_val=Search_for_book_from_library(lib_ptr,conn_sock,&lock,log_ptr);
							if(ret_val==NULL_POINTER_ACCESS_ERROR)
							{
								ONLY_SEND(conn_sock,"Error Accessing Pointers.Please try again.")
							}
							else if(ret_val==POINTER_IS_NULL)
							{
								ONLY_SEND(conn_sock,"Pointer is NULL.Please try again.")
							}
							else
							{
								ONLY_SEND(conn_sock,"ERROR")
							}
						}
						else if(strcmp(function_chosen,"8")==0)		//Exit.
						{
							QUIT_PROG(conn_sock,THANK_YOU_FOR_VISITING)
							return 0;
						}


						//Updating the Admin struct into the File.
						update_cred_file(curr_adm,NULL,adm_ptr,&lock,sizeof(admin),adm_offset,conn_sock);
					}
					fclose(lib_ptr);
					fclose(lib_req_ptr);
					fclose(cust_ptr);
					fclose(adm_ptr);
				}
				sleep(1);
			}
		}
	}
	return 0;
}

void choose_cred_file(char *buff,char *option)
{
	if(strcmp(option,"1")==0)
	{
		strcpy(buff,ADMIN_CRED_FILE);
	}
	else if(strcmp(option,"2")==0)
	{
		strcpy(buff,CUSTOMER_CRED_FILE);
	}
}

long long int is_entity_in_file(admin *cred_adm,admin *read_adm,customer *cred_cust,customer *read_cust,int entity_type,FILE *ptr,struct flock *lock,int conn_sock)
{
	if(cred_adm==NULL || read_adm==NULL || cred_cust==NULL || read_cust==NULL || ptr==NULL || lock==NULL)
	{
		perror("pointer is null: ");
		return -1;
	}

	//Moving the File pointer to the beginning.
	int mv_lp=fseek(ptr,0,SEEK_SET);
	FSEEK_CHK(mv_lp,conn_sock)

	int is_registered=0;
	do
	{
		int read_stat;
		acquire_read_lock(ptr,lock,conn_sock);
		if(entity_type==1)
		{
			read_stat=fread(read_adm,sizeof(admin),1,ptr);
		}
		else if(entity_type==2)
		{
			read_stat=fread(read_cust,sizeof(customer),1,ptr);
		}
		release_acquired_read_lock(ptr,lock,conn_sock);
		if(read_stat!=1)
		{
			break;
		}

		if(entity_type==1 && strcmp(cred_adm->name,read_adm->name)==0 && strcmp(cred_adm->password,read_adm->password)==0 && strcmp(cred_adm->adminKey,read_adm->adminKey)==0)
		{
			is_registered=1;
			break;
		}
		else if(entity_type==2 && strcmp(cred_cust->name,read_cust->name)==0 && strcmp(cred_cust->password,read_cust->password)==0)
		{
			is_registered=1;
			break;
		}
	}while(1);

	long long int ret_val=-1;
	int rec_size=(entity_type==1)?sizeof(admin):sizeof(customer);
	if(is_registered)
	{
		int mv_lp_back=fseek(ptr,-rec_size,SEEK_CUR);
		FSEEK_CHK(mv_lp_back,conn_sock)
		ret_val=ftell(ptr);
	}
	return ret_val;
}

void choose_num_parse(char *main_option,char *inner_option,int *num_parse)
{
	*num_parse=(strcmp(main_option,"1")==0)?2:7;					//Deciding base value for num_parse based on login/signup.
	*num_parse=(strcmp(inner_option,"2")==0)?*num_parse:((*num_parse)+1);		//Adding '1' to num_parse if option chosen is for Admin.
}

void update_cred_file(admin *curr_adm,customer *curr_cust,FILE *ptr,struct flock *lock,int rec_size,long long int offset,int conn_sock)
{
	if(ptr==NULL || lock==NULL)
	{
		perror("pointer is null: ");
		QUIT_PROG(conn_sock,"ERROR")
		exit(1);
	}

	//Moving location pointer to offset.
	int mv_lp_loc=fseek(ptr,offset,SEEK_SET);
	FSEEK_CHK(mv_lp_loc,conn_sock); 

	//Updating the file to reflect changes in struct.
	int write_stat;
	acquire_write_lock(ptr,lock,conn_sock);
	if(curr_adm!=NULL && curr_cust==NULL)
	{
		write_stat=fwrite(curr_adm,rec_size,1,ptr);
	}
	else if(curr_cust!=NULL && curr_adm==NULL)
	{
		write_stat=fwrite(curr_cust,rec_size,1,ptr);
	}
	release_acquired_write_lock(ptr,lock,conn_sock);
	if(write_stat!=1)
	{
		perror("fwrite(): ");
		QUIT_PROG(conn_sock,"fwrite error")
		exit(1);
	}
}
