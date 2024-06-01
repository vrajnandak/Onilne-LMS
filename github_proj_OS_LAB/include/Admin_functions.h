#ifndef ADMIN_FUNCTIONS_H
#define ADMIN_FUNCTIONS_H

#include"common_utils.h"
#include"homepage.h"
#include"structures.h"
#include"locking.h"
#include"Customer_functions.h"
#include"server.h"
#include"logging.h"

#define ADMIN_FUNCTIONS "What would you like to do\n1 - Add a new Book to the Library\n2 - Delete a Book from Library\n3 - Update a Book in the Library\n4 - View all Customers in OLMS\n5 - View your profile\n6 - View the Library\n7 - Search for Book in Library\n8 - Exit Library"

//Macros
#define ADD_SUCCESS 0				//Indicates that Adding a new Book to the Library was successfull.

#define DELETE_SUCCESS 0			//Indicates that Deleting a Book from the Library was successfull.
#define DELETE_CANT_BE_DONE 2			//Indicates that there is a copy of the Book being borrowed by a Customer.

#define UPDATE_SUCCESS 0			//Indicates that Updating the Book from the Library was successfull.
#define UPDATE_CANT_BE_DONE 2			//Indicates that atleast one copy of the Book is being borrowed by a Customer.

#define SEARCH_SUCCESS 0			//Indicates that the Search operation was successfull.
#define CHOOSE_EXISTING_SEARCH_OPTION 1		//Indicates that a Search option different from the given options has been Chosen.
#define NO_SEARCH_HITS 2			//Indicates that there were no Books satisfying the Search Conditions.

#define NO_BOOKS_IN_LIBRARY 5			//Indicates that the Library is empty.


int Add_new_book_to_library(FILE *lib_req_ptr,FILE *lib_ptr,int conn_sock,struct flock *lock);	//Function to add a new book to the library.
int Delete_book_from_library(FILE *lib_ptr,int conn_sock,struct flock *lock,FILE *cust_ptr);	//Function to delete a book from the library. Cannot delete a book if even one of the copies of the book is currently being borrowed by a user.
int Update_book_from_library(FILE *lib_ptr,FILE *cust_ptr,int conn_sock,struct flock *lock);	//Function to update details of an existing book. Not possible to change the name of a book or the ISBN of book or it's price if even one of it's copies is being borrowed by a user.
void View_library(FILE *lib_ptr,int conn_sock,struct flock *lock);				//Views all the books in the library.
int Search_for_book_from_library(FILE *lib_ptr,int conn_sock,struct flock *lock,FILE *log_ptr);	//Function to search for a book based on a particular field value of the book.
void Print_all_customers(FILE *cust_ptr,int conn_sock,struct flock *lock);			//Function to go through all the customers that have signed up in the OLMS.
void View_profile_adm(int conn_sock,admin *adm);						//Function to view profile.

#endif
