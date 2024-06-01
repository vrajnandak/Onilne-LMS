#include"../include/Admin_functions.h"

//Function to add a new book to the library.
int Add_new_book_to_library(FILE *lib_req_ptr,FILE *lib_ptr,int conn_sock,struct flock *lock)
{
	IS_STRUCT_POINTER_NULL(lib_req_ptr,"Add_new_book_to_library()","lib_req_ptr")
	IS_STRUCT_POINTER_NULL(lib_ptr,"Add_new_book_to_library()","lib_ptr")
	IS_STRUCT_POINTER_NULL(lock,"Add_new_book_to_library()","lock")

	//Declaring variables to read the Library_request.bin file.
	int deleted_flag;
	book *req_book=(book*)malloc(sizeof(book));
	MALLOC_FAIL_CHECK(req_book,"Add_new_book_to_library()","book")

	//Setting the location pointer to the beginning of the file with the book requests.
	int mv_lp=fseek(lib_req_ptr,0,SEEK_SET);
	FSEEK_CHECK(mv_lp,"in Add_new_book_to_library(), fseek() by offset: 0 from SEEK_SET")

	//Iterativey reading through the Library_request file to search for the book to be added. Overwrite the flag to '1' after finding the book.
	int req_book_found=0;
	do
	{
		int shd_break=0;
		acquire_read_lock(lib_req_ptr,lock,conn_sock);				//Acquiring the read lock.
		int read_status=fread(&deleted_flag,sizeof(int),1,lib_req_ptr);		//Reading the flag.
		if(read_status!=1)
		{
			shd_break=1;
		}
		read_status=fread(req_book,sizeof(book),1,lib_req_ptr);			//Reading the book.
		release_acquired_read_lock(lib_req_ptr,lock,conn_sock);
		if(shd_break || read_status!=1)
		{
			printf("breaking out because reached end\n");
			break;
		}

		if(deleted_flag==0)							//Overwriting the flag to be '1'.
		{
			//Overwriting the deleted flag to mark it as '1'.
			req_book_found=1;
			deleted_flag=1;
			int mv_lrp=fseek(lib_req_ptr,-sizeof(int)-sizeof(book),SEEK_CUR);
			FSEEK_CHECK(mv_lrp,"in Add_new_book_to_library(), fseek() by offset: -sizeof(int)-sizeof(book) from SEEK_CUR")
			acquire_write_lock(lib_req_ptr,lock,conn_sock);
			int write_stat=fwrite(&deleted_flag,sizeof(int),1,lib_req_ptr);
			release_acquired_write_lock(lib_req_ptr,lock,conn_sock);
			if(write_stat!=1)
			{
				return SYSTEM_ERROR;
			}
			break;
		}
	}while(1);

	//Add book to the Library if a Book to be added is found.
	if(req_book_found)
	{
		int read_flag;
		book *read_book=(book*)malloc(sizeof(book));
		MALLOC_FAIL_CHECK(read_book,"Add_new_book_to_library()","book")

		//Iteratively reading through the file to try to Replace a Deleted Book, else append the Book.
		do
		{
			int shd_break=0;
			acquire_read_lock(lib_ptr,lock,conn_sock);
			int read_stat=fread(&read_flag,sizeof(int),1,lib_ptr);
			if(read_stat!=1)
			{
				shd_break=1;
			}
			read_stat=fread(read_book,sizeof(book),1,lib_ptr);
			release_acquired_read_lock(lib_ptr,lock,conn_sock);
			if(read_stat!=1 || shd_break)
			{
				break;
			}

			if(read_flag==1)
			{
				int mv_lp_back=fseek(lib_ptr,-sizeof(int)-sizeof(book),SEEK_CUR);
				FSEEK_CHECK(mv_lp_back,"in Add_new_book_to_library(), fseek() by offset: -sizeof(int)-sizeof(curr) from SEEK_CUR")
				break;
			}
		}while(1);

		//Adding a Requested Book to the Library.
		int flag=0;
		int sys_error=0;
		acquire_write_lock(lib_ptr,lock,conn_sock);
		int write_stat=fwrite(&flag,sizeof(int),1,lib_ptr);
		if(write_stat!=1)
		{
			sys_error=1;
		}
		write_stat=fwrite(req_book,sizeof(book),1,lib_ptr);
		release_acquired_write_lock(lib_ptr,lock,conn_sock);
		if(write_stat!=1 || sys_error)
		{
			return SYSTEM_ERROR;
		}
		return ADD_SUCCESS;
	}

	return BOOK_NOT_FOUND;
}

//Function to delete a book from the library. Cannot delete a book if even one of the copies of the book is currently being borrowed by a user.
int Delete_book_from_library(FILE *lib_ptr,int conn_sock,struct flock *lock,FILE *cust_ptr)
{
	IS_STRUCT_POINTER_NULL(lib_ptr,"Delete_book_from_library()","lib_ptr")
	IS_STRUCT_POINTER_NULL(lock,"Delete_book_from_library()","lock")
	IS_STRUCT_POINTER_NULL(cust_ptr,"Delete_book_from_library()","cust_ptr")

	//Receiving the ISBN of the Book to be Deleted from the Library.
	char isbn[BUFFER_SIZE];
	char *endptr;
	send(conn_sock,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);
	send(conn_sock,"START SCAN?Please enter the ISBN code of the book you would like to delete from the library.?END SCAN",BUFFER_SIZE,0);
	recv(conn_sock,isbn,BUFFER_SIZE,0);
	long long int delete_isbn=strtoll(isbn,&endptr,10);

	//Checking if any copy of the Book is currently being borrowed by any Customer.
	customer *read_cust=(customer*)malloc(sizeof(customer));
	MALLOC_FAIL_CHECK(read_cust,"Delete_book_from_library()","customer")
	book *tmp_book=(book*)malloc(sizeof(book));
	MALLOC_FAIL_CHECK(tmp_book,"Delete_book_from_library()","book")
	tmp_book->ISBN=delete_isbn;
	int mv_cp=fseek(cust_ptr,0,SEEK_SET);
	FSEEK_CHECK(mv_cp,"in Delete_book_from_library(), fseek() by offset: 0 from SEEK_SET")
	do
	{
		acquire_read_lock(cust_ptr,lock,conn_sock);
		int read_stat=fread(read_cust,sizeof(customer),1,cust_ptr);
		release_acquired_read_lock(cust_ptr,lock,conn_sock);
		if(read_stat!=1)
		{
			break;
		}

		int ret_val=customer_has_book(read_cust,tmp_book);
		if(ret_val!=CUSTOMER_DOES_NOT_HAVE_THE_BOOK)
		{
			return DELETE_CANT_BE_DONE;
		}
	}while(1);

	//Reading the Library to find the Book to be Deleted.
	int read_flag;
	book *read_book=(book*)malloc(sizeof(book));
	MALLOC_FAIL_CHECK(read_book,"Delete_book_from_library()","book")
	int mv_lp=fseek(cust_ptr,0,SEEK_SET);
	FSEEK_CHECK(mv_lp,"in Delete_book_from_library(), fseek() by offset: 0 from SEEK_SET")
	int books_in_lib=0;
	do
	{
		int shd_break=0;
		acquire_read_lock(lib_ptr,lock,conn_sock);
		int read_stat=fread(&read_flag,sizeof(int),1,lib_ptr);
		if(read_stat!=1)
		{
			shd_break=1;
		}
		read_stat=fread(read_book,sizeof(book),1,lib_ptr);
		release_acquired_read_lock(lib_ptr,lock,conn_sock);
		if(shd_break || read_stat!=1)
		{
			break;
		}

		if(read_flag==0)
		{
			books_in_lib++;
			if(read_book->ISBN==delete_isbn)
			{
				//Overwriting the book.
				int mv_lp_back=fseek(lib_ptr,-sizeof(int)-sizeof(book),SEEK_CUR);
				FSEEK_CHECK(mv_lp_back,"in Delete_book_from_library(), fseek() by offset: -sizeof(int)-sizeof(book) from SEEK_CUR")
				int delete_flag=1;
				int write_stat=fwrite(&delete_flag,sizeof(book),1,lib_ptr);
				if(write_stat!=1)
				{
					return SYSTEM_ERROR;
				}
				return DELETE_SUCCESS;
			}
		}
	}while(1);

	if(books_in_lib==0)
	{
		return NO_BOOKS_IN_LIBRARY;
	}

	return BOOK_NOT_FOUND;
}

//Function to update details of an existing book. Not possible to change the name of a book or the ISBN of book or it's price if even one of it's copies is being borrowed by a user.
int Update_book_from_library(FILE *lib_ptr,FILE *cust_ptr,int conn_sock,struct flock *lock)
{
	IS_STRUCT_POINTER_NULL(lib_ptr,"Update_book_from_library()","lib_ptr")
	IS_STRUCT_POINTER_NULL(cust_ptr,"Update_book_from_library()","cust_ptr")
	IS_STRUCT_POINTER_NULL(lock,"Update_book_from_library()","lock")

	//Receiving the Fields to be Updated for the Book.
	char book_creds[BUFFER_SIZE];
	send(conn_sock,EXECUTE_COMMAND_LINE,OPTION_SIZE,0);
	send(conn_sock,"START SCAN?Please enter the isbn code of the book you would like to update the fields of?Enter the new quantity of the book,Enter 'NO' if you don't want to update it?Enter the new price of the book,Enter 'NO' if you don't want to update it?Enter the new title of the book, Enter 'NO' if you don't want to update it?Enter the new author of the book, Enter 'NO' if you don't want to update it?END SCAN",BUFFER_SIZE,0);
	recv(conn_sock,book_creds,BUFFER_SIZE,0);

	//Parsing the ISBN of the book to be updated.
	char *token=strtok(book_creds,"?");
	char *endptr;
	long long int update_isbn=strtoll(token,&endptr,10);

	//Checking if any copy of the Book is currently being borrowed by any Customer.
	customer *read_cust=(customer*)malloc(sizeof(customer));
	MALLOC_FAIL_CHECK(read_cust,"Delete_book_from_library()","customer")
	book *tmp_book=(book*)malloc(sizeof(book));
	MALLOC_FAIL_CHECK(tmp_book,"Delete_book_from_library()","book")
	tmp_book->ISBN=update_isbn;
	int mv_cp=fseek(cust_ptr,0,SEEK_SET);
	FSEEK_CHECK(mv_cp,"in Delete_book_from_library(), fseek() by offset: 0 from SEEK_SET")
	do
	{
		acquire_read_lock(cust_ptr,lock,conn_sock);
		int read_stat=fread(read_cust,sizeof(customer),1,cust_ptr);
		release_acquired_read_lock(cust_ptr,lock,conn_sock);
		if(read_stat!=1)
		{
			break;
		}

		int ret_val=customer_has_book(read_cust,tmp_book);
		if(ret_val!=CUSTOMER_DOES_NOT_HAVE_THE_BOOK)
		{
			return UPDATE_CANT_BE_DONE;
		}
	}while(1);

	//Reading the Library to find the Book to be Deleted.
	int read_flag;
	book *read_book=(book*)malloc(sizeof(book));
	MALLOC_FAIL_CHECK(read_book,"Update_book_from_library()","book")
	int books_in_lib=0;
	do
	{
		int shd_break=0;
		acquire_read_lock(lib_ptr,lock,conn_sock);			//Acquiring the Read lock.
		int read_stat=fread(&read_flag,sizeof(int),1,lib_ptr);		//Reading the is_delete flag.
		if(read_stat!=1)
		{
			shd_break=1;
		}
		read_stat=fread(read_book,sizeof(book),1,lib_ptr);		//Reading the Book.
		release_acquired_read_lock(lib_ptr,lock,conn_sock);
		if(shd_break || read_stat!=1)
		{
			break;
		}

		if(read_flag==0)
		{
			books_in_lib++;
			if(read_book->ISBN==update_isbn)			//Overwrite the Book in the Library with the Updated Copy.
			{
				int fields_update=0;
				token=strtok(NULL,"?");
				if(strcmp(token,"NO")!=0)
				{
					read_book->quantity=atoi(token);		//Updating the Quantity field.
					fields_update++;
				}

				token=strtok(NULL,"?");
				if(strcmp(token,"NO")!=0)
				{
					read_book->price=atoi(token);
					fields_update++;
				}

				token=strtok(NULL,"?");
				if(strcmp(token,"NO")!=0)
				{
					strcpy(read_book->title,token);			//Updating the Title field.
					fields_update++;
				}

				token=strtok(NULL,"?");
				if(strcmp(token,"NO")!=0)
				{
					strcpy(read_book->author,token);		//Updating the Author field.
					fields_update++;
				}

				if(fields_update!=0)
				{
					int mv_lp_book=fseek(lib_ptr,-sizeof(book),SEEK_CUR);
					FSEEK_CHECK(mv_lp_book,"in Update_book_from_library(), fseek() by offset: -sizeof(book) from SEEK_CUR")
					acquire_write_lock(lib_ptr,lock,conn_sock);
					int write_stat=fwrite(read_book,sizeof(book),1,lib_ptr);
					release_acquired_write_lock(lib_ptr,lock,conn_sock);
					if(write_stat!=1)
					{
						return SYSTEM_ERROR;
					}
				}

				return UPDATE_SUCCESS;
			}
		}
	}while(1);

	if(books_in_lib==0)
	{
		return NO_BOOKS_IN_LIBRARY;
	}

	return BOOK_NOT_FOUND;
}

//Function to search for a book based on a particular field value of the book.
int Search_for_book_from_library(FILE *lib_ptr,int conn_sock,struct flock *lock,FILE *log_ptr)
{
	return search_book(lib_ptr,conn_sock,lock,log_ptr);
}

//Function to go through all the customers that have signed up in the OLMS.
void Print_all_customers(FILE *cust_ptr,int conn_sock,struct flock *lock)
{
	IS_STRUCT_POINTER_NULL(cust_ptr,"Print_all_customers()","cust_ptr")
	IS_STRUCT_POINTER_NULL(lock,"Print_all_customers()","lock")

	//Reading through the Customer file.
	customer *read_cust=(customer*)malloc(sizeof(customer));
	MALLOC_FAIL_CHECK(read_cust,"Print_all_customers()","customer")
	do
	{
		acquire_read_lock(cust_ptr,lock,conn_sock);
		int read_status=fread(read_cust,sizeof(customer),1,cust_ptr);
		release_acquired_read_lock(cust_ptr,lock,conn_sock);
		if(read_status!=1)
		{
			break;
		}

		view_profile_cust(conn_sock,read_cust);
	}while(1);
}

//Function to view profile.
void View_profile_adm(int conn_sock,admin *adm)
{
	IS_STRUCT_POINTER_NULL(adm,"View_profile_adm()","adm")

	//Sending the Admin information.
	char adm_info[BUFFER_SIZE];
	snprintf(adm_info,BUFFER_SIZE,"Viewing Profile:\nName: %s\nAge: %d\nGender: %d\nEmail: %s\nPhone: %s\nDate-of-Birth: %s\nPassword: %s\nAdmin Key: %s\n",adm->name,adm->age,adm->gender,adm->email,adm->phone,adm->DOB,adm->password,adm->adminKey);
	send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
	send(conn_sock,adm_info,BUFFER_SIZE,0);
}

void View_library(FILE *lib_ptr,int conn_sock,struct flock *lock)
{
	view_library(lib_ptr,conn_sock,lock);
}
