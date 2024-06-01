#ifndef CUSTOMER_FUNCTIONS_H
#define CUSTOMER_FUNCTIONS_H

#include"common_utils.h"
#include"structures.h"
#include"server.h"
#include"locking.h"
#include"logging.h"

#define CUSTOMER_FUNCTIONS "What would you like to do\n1 - View all Books in the Library\n2 - Borrow a Book from the library\n3 - Return a Book\n4 - Request to add a Book to the library\n5 - View your profile\n6 - Search for a book\n7 - Exit from OLMS.\n"
#define MAX_BOOKS_BORROWED_MSG "You have borrowed maximum number of books. You cannot borrow any more books."
#define IS_STRUCT_POINTER_NULL(PTR,FUNCTION_NAME,PTR_NAME) if(PTR==NULL){printf("in %s, %s is NULL\n",FUNCTION_NAME,PTR_NAME);}
#define STRTOK_POINTER_CHECK(PTR,FUNCTION_NAME,PTR_NAME) if(PTR==NULL){printf("in %s, %s is NULL\n",FUNCTION_NAME,PTR_NAME); return NULL_POINTER_ACCESS_ERROR;}
#define FSEEK_CHECK(RET_VAL,MSG) if(RET_VAL!=0){perror(MSG); return SYSTEM_ERROR;}

//Macros
#define BOOK_BORROW_SUCCESS 0			//Indicates that the book was borrowed successfully by the Customer.
#define CANT_BORROW_ANOTHER_BOOK 2		//Indicates the Customer cannot borrow another book due to the limit on number of books he can borrow.
#define BOOK_CANT_BE_BORROWED 3			//Indicates that there is no copy of the book currently available in the Library.

#define BOOK_RETURN_SUCCESS 0			//Indicates that Customer was able to successfully return the book.
#define NO_BOOK_TO_RETURN 1			//Indicates that the Customer is Trying to return a book despite not having borrowed any books.
#define BOOK_RETURN_ERROR 2			//Indicates that there was an error in the Fields entered by client for returning the book.

#define BOOK_REQUEST_SUCCESS 0			//Indicates that the Customer was able to successfully request a book to be added to the Library.

#define SEARCH_SUCCESS 0			//Indicats that the search was successfull.
#define SEARCH_NOT_PRESENT 1			//Indicates that the Search option is not present.

#define CUSTOMER_DOES_NOT_HAVE_THE_BOOK -1	//Indicates that the Customer hasn't already borrowed that book.
#define BOOK_NOT_FOUND 10			//Indicates that the book was not found in Library based on some search conditions.
#define SYSTEM_ERROR 11				//Indicates that there was an error(maybe malloc) borrowing the book and hence the book was not borrowed.

//Customer Functions
void view_library(FILE *lib_ptr,int conn_sock,struct flock *lock);				//Views all the books in the library.
int borrow_book(FILE *lib_ptr,int conn_sock,customer *cust,struct flock *lock,FILE *log_ptr);	//Borrows a book for the given customer.
int return_book(FILE *lib_ptr,int conn_sock,customer *cust,struct flock *lock,FILE *log_ptr);	//Returns a book borrowed by the given customer.
int request_add_book(FILE *lib_req_ptr,int conn_sock,struct flock *lock,FILE *log_ptr);		//Requests to add a book to the library.
void view_profile_cust(int conn_sock,customer *cust);						//Views the profile of the given customer.
int search_book(FILE *lib_ptr,int conn_sock,struct flock *lock,FILE *log_ptr);			//Searches for a book based on a value from the library.

#endif
