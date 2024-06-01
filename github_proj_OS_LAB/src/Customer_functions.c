#include"../include/Customer_functions.h"

void view_library(FILE *lib_ptr,int conn_sock,struct flock *lock)
{
	IS_STRUCT_POINTER_NULL(lib_ptr,"view_library()","lib_ptr")
	IS_STRUCT_POINTER_NULL(lock,"view_library()","lock")

	book *read_book=(book*)malloc(sizeof(book));						//Individual book read from library.
	MALLOC_FAIL_CHECK(read_book,"view_library()","book")
	
	//Sending the library column fields.
	char columns[BUFFER_SIZE];
	get_library_columns(columns);
	send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
	send(conn_sock,columns,BUFFER_SIZE,0);

	//Reading the books iteratively from the library.
	int read_flag;
	int books_read=0;
	do
	{
		int shd_break=0;

		acquire_read_lock(lib_ptr,lock,conn_sock);				//Acquiring the read lock.
		int read_stat=fread(&read_flag,sizeof(int),1,lib_ptr);			//Reading the flag.
		if(read_stat!=1)
		{
			shd_break=1;
		}

		read_stat=fread(read_book,sizeof(book),1,lib_ptr);			//Reading the book.
		release_acquired_read_lock(lib_ptr,lock,conn_sock);			//Releasing the read lock.

		if(shd_break==1 || read_stat!=1)			//Breaking out of the loop if EOF is reached or an error while reading the file.
		{
			break;
		}

		if(!read_flag)
		{
			//Sending the book information to the Customer.
			char book_information[BUFFER_SIZE];
			extract_book_info(book_information,read_book);
			send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
			send(conn_sock,book_information,BUFFER_SIZE,0);
		}

		books_read++;
	}while(1);

	if(!books_read)
	{
		send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
		send(conn_sock,"The Library Currently has no books. Please request an Admin to Add books to the Library.",BUFFER_SIZE,0);
	}
}
int borrow_book(FILE *lib_ptr,int conn_sock,customer *cust,struct flock *lock,FILE *log_ptr)
{
	IS_STRUCT_POINTER_NULL(lib_ptr,"borrow_book()","lib_ptr")
	IS_STRUCT_POINTER_NULL(cust,"borrow_book()","cust")
	IS_STRUCT_POINTER_NULL(lock,"borrow_book()","lock")

	//Client can borrow only a certain number of books.
	if(cust->books_borrowed_count>=MAX_BOOKS_BORROWED)
	{
		send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
		send(conn_sock,MAX_BOOKS_BORROWED_MSG,BUFFER_SIZE,0);
		add_log(cust->name,2,"Borrowing Book from Library",NULL,CANT_BORROW_ANOTHER_BOOK,log_ptr,lock,conn_sock);
		return CANT_BORROW_ANOTHER_BOOK;
	}

	//Receiving the credential fields needed to borrow the book to the Customer.
	char book_creds[BUFFER_SIZE];
	send(conn_sock,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);
	send(conn_sock,"START SCAN?Please Enter the title of the book you would like to borrow?Enter the ISBN code of the book?END SCAN",BUFFER_SIZE,0);
	recv(conn_sock,book_creds,BUFFER_SIZE,0);

	//Extracting the book information from client response.
	char book_title[TITLE_LENGTH];
	long long int book_isbn;
	char *endptr;

	char *token=strtok(book_creds,"?");
	STRTOK_POINTER_CHECK(token,"borrow_book()","token")
	strcpy(book_title,token);
	token=strtok(NULL,"?");
	STRTOK_POINTER_CHECK(token,"borrow_book()","token")
	book_isbn=strtoll(token,&endptr,10);


	book *read_book=(book*)malloc(sizeof(book));						//Individual book read from library.
	MALLOC_FAIL_CHECK(read_book,"borrow_book()","book")

	//Iteratively searching for the book in the library.
	int read_flag;
	int books_read_count=0;
	int book_found=0;
	do
	{
		int shd_break=0;

		acquire_read_lock(lib_ptr,lock,conn_sock);				//Acquiring the read lock.
		int read_stat=fread(&read_flag,sizeof(int),1,lib_ptr);			//Reading the is_deleted flag.
		if(read_stat!=1)
		{
			shd_break=1;
		}

		read_stat=fread(read_book,sizeof(book),1,lib_ptr);			//Reading the book.
		release_acquired_read_lock(lib_ptr,lock,conn_sock);			//Releasing the read lock.

		if(shd_break==1 || read_stat!=1)
		{
			break;
		}

		//Borrow the book only if the ISBN matches and the Title is the same as that required by the Customer.
		if(read_flag==0)
		{
			books_read_count++;

			if(book_isbn==read_book->ISBN)
			{
				if(strcmp(book_title,read_book->title)==0)
				{
					if(read_book->quantity==0)
					{
						add_log(cust->name,2,"Borrowing Book from Library",NULL,BOOK_CANT_BE_BORROWED,log_ptr,lock,conn_sock);
						return BOOK_CANT_BE_BORROWED;
					}
					book_found=1;
				}
				else
				{
					char is_book_correct[OPTION_SIZE];
					send(conn_sock,CHOOSE_ONE_OPTION_SEND,OPTION_SIZE,0);
					send(conn_sock,"The Title you have entered doesn't match exactly with the Title of the book(with the same isbn) in the Library. Is this the book you are looking for?Enter (Y/N)",BUFFER_SIZE,0);
					recv(conn_sock,is_book_correct,OPTION_SIZE,0);

					if(strcmp(is_book_correct,"Y")==0)
					{
						book_found=1;
					}
					else
					{
						add_log(cust->name,2,"Borrowing Book from Library",NULL,BOOK_NOT_FOUND,log_ptr,lock,conn_sock);
						return BOOK_NOT_FOUND;
					}
				}
			}

			if(book_found==1)
			{
				break;
			}
		}
	}while(1);

	if(book_found)
	{
		//Overwriting the book after updating the quantity into the Library.
		read_book->quantity=read_book->quantity-1;				//Decrementing the quantity of the book by 1.
		int mv_lp=fseek(lib_ptr,-sizeof(book),SEEK_CUR);
		FSEEK_CHECK(mv_lp,"in borrow_book(), fseek() by offset: -sizeof(book) from SEEK_CUR")
		acquire_write_lock(lib_ptr,lock,conn_sock);
		int book_write_stat=fwrite(read_book,sizeof(book),1,lib_ptr);
		release_acquired_write_lock(lib_ptr,lock,conn_sock);
		if(book_write_stat!=1)
		{
			printf("in borrow_book(), fwrite() of read_book");
			add_log(cust->name,2,"Borrowing Book from Library",NULL,CANT_BORROW_ANOTHER_BOOK,log_ptr,lock,conn_sock);
			return SYSTEM_ERROR;
		}

		//Borrowing the book by adding it into the Customer's list of borrowed books. A borrow limit of 7 days.
		read_book->quantity=1;
		(cust->books_borrowed[cust->books_borrowed_count]).borrowedbook=*read_book;
		time_t current_time=time(NULL);
		(cust->books_borrowed[cust->books_borrowed_count]).borrow_date=current_time;
		(cust->books_borrowed[cust->books_borrowed_count]).return_date=current_time+(7 * 24 * 60 * 60);
		cust->books_borrowed_count=cust->books_borrowed_count+1;

		add_log(cust->name,2,"Borrowing Book from Library",&(cust->books_borrowed[(cust->books_borrowed_count)-1]),CANT_BORROW_ANOTHER_BOOK,log_ptr,lock,conn_sock);
		return BOOK_BORROW_SUCCESS;

		/**********USE BELOW CODE INSTEAD IF YOU WANT TO MAINTAIN BOOKS HAVING SAME ISBN IN TERMS OF QUANTITY INSTEAD OF DIFFERENT ENTRIES*******
		int ret_val=customer_has_book(cust,read_book);
		if(ret_val!=CUSTOMER_DOES_NOT_HAVE_THE_BOOK)		//Indicates that the Customer hasn't previously borrowed the book.
		{
			read_book->quantity=1;
			cust->books_borrowed[cust->books_borrowed_count]=*read_book;
			cust->books_borrowed_count=cust->books_borrowed_count+1;

			return BOOK_BORROW_SUCCESS;
		}
		else							//Indicates that the Customer has previously borrowed the book.
		{
			(cust->books_borrowed[ret_val]).quantity=((cust->books_borrowed[ret_val]).quantity)+1;
			return BOOK_BORROW_SUCCESS;
		}
		*/
	}

	add_log(cust->name,2,"Borrowing Book from Library",NULL,BOOK_NOT_FOUND,log_ptr,lock,conn_sock);
	return BOOK_NOT_FOUND;
}
int return_book(FILE *lib_ptr,int conn_sock,customer *cust,struct flock *lock,FILE *log_ptr)
{
	IS_STRUCT_POINTER_NULL(lib_ptr,"return_book()","lib_ptr")
	IS_STRUCT_POINTER_NULL(cust,"return_book()","cust")
	IS_STRUCT_POINTER_NULL(lock,"return_book()","lock")

	if(cust->books_borrowed_count<=0)				//Returning an error code if the Customer has not borrowed any book.
	{
		perror("Cannot return book as No book has been borrowed yet");
		add_log(cust->name,2,"Returning Book to Library",NULL,NO_BOOK_TO_RETURN,log_ptr,lock,conn_sock);
		return NO_BOOK_TO_RETURN;
	}

	//Receiving the Credentials of the Book to be returned.
	char book_creds[BUFFER_SIZE];
	send(conn_sock,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);
	send(conn_sock,"START SCAN?Please Enter the title of the book you would like to borrow?Enter the ISBN code of the book?END SCAN",BUFFER_SIZE,0);
	recv(conn_sock,book_creds,BUFFER_SIZE,0);
	printf("recvd creds: %sEND\n",book_creds);

	//Extracting the book information from client response.
	char book_title[TITLE_LENGTH];
	long long int book_isbn;
	char *endptr;

	char *token=strtok(book_creds,"?");
	STRTOK_POINTER_CHECK(token,"return_book()","token")
	strcpy(book_title,token);
	token=strtok(NULL,"?");
	STRTOK_POINTER_CHECK(token,"return_book()","token")
	book_isbn=strtoll(token,&endptr,10);

	int read_flag;
	book *read_book=(book*)malloc(sizeof(book));			//To iteratively read the books from the Library.
	MALLOC_FAIL_CHECK(read_book,"return_book()","book")

	strcpy(read_book->title,book_title);
	read_book->ISBN=book_isbn;

	int ret_val=customer_has_book(cust,read_book);			//Checking to see if the customer has the book.
	printf("customer_has_book: %d\n",ret_val);
	if(ret_val==CUSTOMER_DOES_NOT_HAVE_THE_BOOK)
	{
		add_log(cust->name,2,"Returning Book to Library",NULL,CUSTOMER_DOES_NOT_HAVE_THE_BOOK,log_ptr,lock,conn_sock);
		return CUSTOMER_DOES_NOT_HAVE_THE_BOOK;
	}

	//Reading iteratively through the library.
	do
	{
		int shd_break=0;

		//Reading the flag and the book.
		acquire_read_lock(lib_ptr,lock,conn_sock);
		int read_stat=fread(&read_flag,sizeof(int),1,lib_ptr);
		if(read_stat!=1)
		{
			shd_break=1;
		}
		read_stat=fread(read_book,sizeof(book),1,lib_ptr);
		if(read_stat!=1 || shd_break==1)
		{
			break;
		}

		if(read_flag==0)
		{
			if(book_isbn==read_book->ISBN)
			{
				printf("a match in the ISBN's of the book to be returned.\n");
				if(strcmp(book_title,read_book->title)==0)
				{
					printf("a match in the titles of the book to be deleted and the book read. read_book title: %s\n",read_book->title);
					//Overwriting the quantity of the book in the library.
					read_book->quantity=read_book->quantity+1;
					acquire_write_lock(lib_ptr,lock,conn_sock);
					int mv_lp=fseek(lib_ptr,-sizeof(book),SEEK_CUR);
					FSEEK_CHECK(mv_lp,"in return_book(), fseek() by offset: -sizeof(book) from SEEK_CUR")
					int write_stat=fwrite(read_book,sizeof(book),1,lib_ptr);
					release_acquired_write_lock(lib_ptr,lock,conn_sock);
					if(write_stat!=1)
					{
						perror("in return_book(), fwrite() of book: ");
						add_log(cust->name,2,"Returning Book to Library",NULL,SYSTEM_ERROR,log_ptr,lock,conn_sock);
						return SYSTEM_ERROR;
					}

					//Returning the book.
					bbook *empty_bbook=(bbook*)malloc(sizeof(bbook));
					MALLOC_FAIL_CHECK(empty_bbook,"return_book()","book")
					bbook *tmp_bbook=(bbook*)malloc(sizeof(bbook));
					MALLOC_FAIL_CHECK(tmp_bbook,"return_book()","book")
					tmp_bbook=&(cust->books_borrowed[ret_val]);

					int j=ret_val;
					for(;j<cust->books_borrowed_count-1;j++)
					{
						(cust->books_borrowed[j]).borrowedbook=(cust->books_borrowed[j+1]).borrowedbook;
					}
					cust->books_borrowed[j]=*empty_bbook;
					cust->books_borrowed_count=cust->books_borrowed_count-1;

					add_log(cust->name,2,"Returning Book to Library",tmp_bbook,BOOK_RETURN_SUCCESS,log_ptr,lock,conn_sock);
					return BOOK_RETURN_SUCCESS;
					/********USE BELOW CODE IF MAINTAINING BOOKS WITH SAME ISBN AS A SINGLE ENTRY IN THE BORROWED BOOKS ARRAY******
					if((cust->books_borrowed[ret_val]).borrowedbook.quantity==1)
					{
						book *empty_book=(book*)malloc(sizeof(book));
						MALLOC_FAIL_CHECK(empty_book,"return_book()","book")
						int j=ret_val;
						for(;j<cust->books_borrowed_count-1;j++)
						{
							cust->books_borrowed[j]=cust->books_borrowed[j+1];
						}
						cust->books_borrowed[j]=*empty_book;
					}
					else
					{
						(cust->books_borrowed[ret_val]).quantity=((cust->books_borrowed[ret_val]).quantity)-1;
					}
					return BOOK_RETURN_SUCCESS;
					*/
				}
				else
				{
					add_log(cust->name,2,"Returning Book to Library",NULL,BOOK_RETURN_ERROR,log_ptr,lock,conn_sock);
					return BOOK_RETURN_ERROR;
				}
			}
		}
	}while(1);

	add_log(cust->name,2,"Returning Book to Library",NULL,BOOK_NOT_FOUND,log_ptr,lock,conn_sock);
	return BOOK_NOT_FOUND;
}
int request_add_book(FILE *lib_req_ptr,int conn_sock,struct flock *lock,FILE *log_ptr)
{
	IS_STRUCT_POINTER_NULL(lib_req_ptr,"request_add_book()","lib_ptr")
	IS_STRUCT_POINTER_NULL(lock,"request_add_book()","lock")
	
	//Receiving the fields of the book to be added to the library.
	char book_creds[BUFFER_SIZE];
	send(conn_sock,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);
	send(conn_sock,"START SCAN?Please Enter the ISBN code of the book you would like to request to add to the library?Enter the Title of the book?Enter the Author of the book?Enter the price of the book?END SCAN",BUFFER_SIZE,0);
	recv(conn_sock,book_creds,BUFFER_SIZE,0);

	//Filling in the details of the book into a struct to write to the file.
	book *req_book=(book*)malloc(sizeof(book));
	MALLOC_FAIL_CHECK(req_book,"request_add_book()","book")
	char *endptr;

	char *token=strtok(book_creds,"?");
	STRTOK_POINTER_CHECK(token,"request_add_book()","token")
	req_book->ISBN=strtoll(token,&endptr,10);

	token=strtok(NULL,"?");
	STRTOK_POINTER_CHECK(token,"request_add_book()","token")
	strcpy(req_book->title,token);
	
	token=strtok(NULL,"?");
	STRTOK_POINTER_CHECK(token,"request_add_book()","token")
	strcpy(req_book->author,token);

	token=strtok(NULL,"?");
	STRTOK_POINTER_CHECK(token,"request_add_book()","token")
	req_book->price=atoi(token);

	req_book->quantity=1;

	//Writing the requested book to the file. Trying to replace a book if there exists a deleted flag.
	int read_flag;
	book *read_book=(book*)malloc(sizeof(book));
	MALLOC_FAIL_CHECK(read_book,"view_library()","book")
	do
	{
		int shd_break=0;
		acquire_read_lock(lib_req_ptr,lock,conn_sock);
		int read_stat=fread(&read_flag,sizeof(int),1,lib_req_ptr);
		if(read_stat!=1)
		{
			shd_break=1;
		}
		read_stat=fread(read_book,sizeof(book),1,lib_req_ptr);
		release_acquired_read_lock(lib_req_ptr,lock,conn_sock);
		if(shd_break || read_stat!=1)
		{
			break;
		}

		if(read_flag==1)
		{
			int mv_lp_back=fseek(lib_req_ptr,-sizeof(int)-sizeof(book),SEEK_CUR);
			FSEEK_CHECK(mv_lp_back,"in request_add_book(), fseek() by offset: -sizeof(int)-sizeof(book) from SEEK_CUR")
			break;
		}
	}while(1);

	int error_writing=0;
	int is_deleted=0;
	acquire_write_lock(lib_req_ptr,lock,conn_sock);
	int write_stat=fwrite(&is_deleted,sizeof(int),1,lib_req_ptr);
	if(write_stat!=1)
	{
		error_writing=1;
	}
	write_stat=fwrite(req_book,sizeof(book),1,lib_req_ptr);
	release_acquired_write_lock(lib_req_ptr,lock,conn_sock);

	if(error_writing || write_stat!=1)
	{
		add_log(NULL,2,"Requesting to Add Book to Library",NULL,SYSTEM_ERROR,log_ptr,lock,conn_sock);
		return SYSTEM_ERROR;
	}

	add_log(NULL,2,"Requesting to Add Book to Library",NULL,BOOK_REQUEST_SUCCESS,log_ptr,lock,conn_sock);
	return BOOK_REQUEST_SUCCESS;
}
void view_profile_cust(int conn_sock,customer *cust)
{
	IS_STRUCT_POINTER_NULL(cust,"view_profile_cust()","cust")
	
	//Sending the Customer information.
	char profile_msg[BUFFER_SIZE];
	snprintf(profile_msg,BUFFER_SIZE,"Viewing Profile:\nName: %s\nAge: %d\nGender: %d\nEmail: %s\nPhone: %s\nDate-of-Birth: %s\nPassword: %s\n-->Number of Books borrowed: %d\n",cust->name,cust->age,cust->gender,cust->email,cust->phone,cust->DOB,cust->password,cust->books_borrowed_count);
	send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
	send(conn_sock,profile_msg,BUFFER_SIZE,0);

	//Sending the library columns.
	char columns[BUFFER_SIZE];
	get_library_columns(columns);
	send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
	send(conn_sock,columns,BUFFER_SIZE,0);

	//Sending the borrowed books.
	if(cust->books_borrowed_count==0)
	{
		send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
		send(conn_sock,"NO BOOKS BORROWED",BUFFER_SIZE,0);
		return;
	}
	for(int i=0;i<cust->books_borrowed_count;i++)
	{
		bbook borrowed_book=cust->books_borrowed[i];
		char book_info[BUFFER_SIZE];
		append_borrow_return_date(book_info,borrowed_book);
		send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
		send(conn_sock,book_info,BUFFER_SIZE,0);
	}
}

int search_book(FILE *lib_ptr,int conn_sock,struct flock *lock,FILE *log_ptr)
{
	IS_STRUCT_POINTER_NULL(lib_ptr,"search_book()","lib_ptr")
	IS_STRUCT_POINTER_NULL(lock,"search_book()","lock")
	
	//Receiving the book credentials to search for the book.
	char search_option[OPTION_SIZE];
	send(conn_sock,CHOOSE_ONE_OPTION_SEND,OPTION_SIZE,0);
	send(conn_sock,"1 - Search by book title\n2 - Search by author\n3 - Search by ISBN code\n4 - Search by quantity greater than or equal to given amount\n5 - Search by price greater than or equal to given amount",BUFFER_SIZE,0);
	recv(conn_sock,search_option,OPTION_SIZE,0);

	book *sch_book=(book*)malloc(sizeof(book));		//Book to search for.
	MALLOC_FAIL_CHECK(sch_book,"search_book()","book")
	
	int sch_option=atoi(search_option);
	printf("sch_option: %d\n",sch_option);
	switch(sch_option)		//Receiving the credentials based on search option.
	{
		case 1: send(conn_sock,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);
			send(conn_sock,"START SCAN?Enter the book title you would like to search by?END SCAN",BUFFER_SIZE,0);
			char title[BUFFER_SIZE];
			printf("receiving the title\n");
			recv(conn_sock,title,BUFFER_SIZE,0);
			strcpy(sch_book->title,title);
			printf("title: %s\n",title);
			break;
		case 2: send(conn_sock,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);
			send(conn_sock,"START SCAN?Enter the author you would like to search by?END SCAN",BUFFER_SIZE,0);
			char auth[BUFFER_SIZE];
			printf("receiving the author\n");
			recv(conn_sock,auth,BUFFER_SIZE,0);
			strcpy(sch_book->author,auth);
			printf("author: %s\n",auth);
			break;
		case 3: send(conn_sock,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);
			send(conn_sock,"START SCAN?Enter the ISBN code you would like to search by?END SCAN",BUFFER_SIZE,0);
			char isbn[BUFFER_SIZE];
			char *endptr;
			recv(conn_sock,isbn,BUFFER_SIZE,0);
			sch_book->ISBN=strtoll(isbn,&endptr,10);
			break;
		case 4: send(conn_sock,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);
			send(conn_sock,"START SCAN?Enter the quantity you would like to search books having greater than or equal to that amount?END SCAN",BUFFER_SIZE,0);
			char quant[BUFFER_SIZE];
			recv(conn_sock,quant,BUFFER_SIZE,0);
			sch_book->quantity=atoi(quant);
			break;
		case 5: send(conn_sock,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);
			send(conn_sock,"START SCAN?Enter the price you would like to search books by having greater than or equal to that price?END SCAN",BUFFER_SIZE,0);
			char price[BUFFER_SIZE];
			recv(conn_sock,price,BUFFER_SIZE,0);
			sch_book->price=atoi(price);
			break;
		default: send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
			 send(conn_sock,"Choose from the Given Options.",BUFFER_SIZE,0);
			 return SEARCH_NOT_PRESENT;
	}

	//Sending the library column fields.
	char columns[BUFFER_SIZE];
	get_library_columns(columns);
	send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
	send(conn_sock,columns,BUFFER_SIZE,0);

	//Searching through the Library iteratively.
	int books_found=0;
	int read_flag;
	book *read_book=(book*)malloc(sizeof(book));
	MALLOC_FAIL_CHECK(read_book,"search_book()","book")
	do
	{
		int shd_break=0;
		acquire_read_lock(lib_ptr,lock,conn_sock);			//Acquiring the read lock.
		int read_stat=fread(&read_flag,sizeof(int),1,lib_ptr);		//Reading the flag.
		if(read_stat!=1)
		{
			shd_break=1;
		}
		read_stat=fread(read_book,sizeof(book),1,lib_ptr);		//Reading the book.
		release_acquired_read_lock(lib_ptr,lock,conn_sock);		//Releasing the lock.
		if(read_stat!=1 || shd_break)
		{
			break;
		}

		if(read_flag==0)
		{
			//Sending the book information if search condition is satisfied.
			int shd_send_book_info=0;
			if(sch_option==1 && (strcmp(read_book->title,sch_book->title)==0))
			{
				shd_send_book_info=1;
			}
			else if(sch_option==2 && (strcmp(read_book->author,sch_book->author)==0))
			{
				shd_send_book_info=1;
			}
			else if(sch_option==3 && (read_book->ISBN==sch_book->ISBN))
			{
				shd_send_book_info=1;
			}
			else if(sch_option==4 && (read_book->quantity>=sch_book->quantity))
			{
				shd_send_book_info=1;
			}
			else if(sch_option==5 && (read_book->price>=sch_book->price))
			{
				shd_send_book_info=1;
			}

			if(shd_send_book_info==1)
			{
				books_found++;
				char book_info[BUFFER_SIZE];
				extract_book_info(book_info,read_book);
				send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
				send(conn_sock,book_info,BUFFER_SIZE,0);
			}
		}
	}while(1);

	if(books_found==0)
	{
		send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
		send(conn_sock,"No Books Match the Given Fields",BUFFER_SIZE,0);
	}
	else
	{
		char books_searched[BUFFER_SIZE];
		snprintf(books_searched,BUFFER_SIZE,"Searches Matched: %d",books_found);
		send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
		send(conn_sock,books_searched,BUFFER_SIZE,0);
	}

	add_log(NULL,2,"Searching Books from Library",NULL,SEARCH_SUCCESS,log_ptr,lock,conn_sock);
	return SEARCH_SUCCESS;
}
