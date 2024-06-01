#ifndef STRUCTURES_H
#define STRUCTURES_H

#include"common_utils.h"

//Macros.
#define MAX_BOOKS_BORROWED 3			//Defines the maximum number of books that can be borrowed.

#define TITLE_LENGTH 	30			//Defines the maximum length of the Title of a Book.
#define AUTHOR_LENGTH 	30			//Defines the maximum length of the Author's name of a Book.

#define NAME_LENGTH 	50			//Defines the maximum length of a Customer's or an Admin's name.
#define EMAIL_LENGTH 	50			//Defines the maximum length of a Customer's or an Admin's email.
#define PHONE_LENGTH 	10			//Defines the maximum length of a Customer's or an Admin's phone.
#define DATE_LENGTH 	15			//Defines the maximum length of a Customer's or an Admin's Date of Birth.
#define PASSWORD_LENGTH 30			//Defines the maximum length of a Customer's or an Amin's password.
#define ADMINKEY_LENGTH 20			//Defines the maximum length of an Admin's adminkey. 
#define ADMINKEY_FORMAT "AdminKey%d"		//Defines the format of what the AdminKey looks like. This string is followed by an integer (1,2 or 3 only).

#define VALIDATE_STRING_LENGTH 8

#define ACTIVITY_LENGTH 40			//Defines the length of the message describing the activity performed in the transaction.

typedef struct Book
{
	long long int ISBN;
	char title[TITLE_LENGTH];
	char author[AUTHOR_LENGTH];
	int quantity;
	int price;
}
book;
//A structure used to maintain the return date of the book when borrowed by the Customer.
typedef struct Borrowed_book
{
	book borrowedbook;
	time_t borrow_date;
	time_t return_date;
}bbook;

typedef struct Customer 
{
	char name[NAME_LENGTH];
	int age;
	int gender;					//M represents 1, F represents 0.
	char email[EMAIL_LENGTH];
	char phone[PHONE_LENGTH];
	char DOB[DATE_LENGTH];
	char password[PASSWORD_LENGTH];
	bbook books_borrowed[MAX_BOOKS_BORROWED];
	int books_borrowed_count;
}customer;

//Admin Key determines the access level of the admin to the OLMS. '1' - view , '2' - add, '3' - update and delete.
typedef struct Admin
{
	char name[NAME_LENGTH];
	int age;
	int gender;					//M represents 1, F represents 0.
	char email[EMAIL_LENGTH];
	char phone[PHONE_LENGTH];
	char DOB[DATE_LENGTH];
	char password[PASSWORD_LENGTH];
	char adminKey[ADMINKEY_LENGTH];			//Of form 'AdminKeyX', where 'X' is either of 1 or 2 or 3.
}admin;

//A Transaction structure used for Logging purpose to record basic Activities in the Library.
typedef struct Transaction
{
	char entity_name[NAME_LENGTH];
	int entity_type;				//1 - represents Admin, 2 - represents Customer.
	char activity_performed[ACTIVITY_LENGTH];
	bbook transaction_book;
	int transaction_state;
}transaction;


//OLMS Client fill functions to extract the user information into the struct.
//void fill_customer(customer *cust,char *information);	//Function to extract customer information and store inside the struct.
//void fill_admin(admin *adm,char *information);		//Function to extract admin information and store inside the struct.
//Function to fill the admin,customer structs with the credential information.
void fill_cred_adm_cust(admin *cred_adm,customer *cred_cust,char *ind_creds[8],int entity_type,int num_parse);

//Fill function to extract book information into the struct.
void extract_book_info(char *buff,book *book);		//Function to extract book information from the book into given character buffer.
void get_library_columns(char *buff);			//Function to print the columns(title,author,isbn,price,quantity) in the OLMS.

int customer_has_book(customer *cust,book *read_book);		//Function to return the index at which the given book has been borrowed by the customer.
void append_borrow_return_date(char *buff,bbook borrowbook);	//Function to append the borrow,return date of book.

#endif
