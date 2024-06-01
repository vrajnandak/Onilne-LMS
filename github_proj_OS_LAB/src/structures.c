#include"../include/structures.h"

//Hard coded function to fill the admin,customer structs with the parsed information to make searching and writing easier.
void fill_cred_adm_cust(admin *cred_adm,customer *cred_cust,char *ind_creds[8],int entity_type,int num_parse)
{
	//Filling the name.
	strcpy(cred_adm->name,ind_creds[0]);
	strcpy(cred_cust->name,ind_creds[0]);

	//Filling the signup credentials.
	if(num_parse==7 || num_parse==8)
	{
		//Filling the age.
		cred_adm->age=atoi(ind_creds[1]);
		cred_cust->age=atoi(ind_creds[1]);

		//Filling the gender.
		cred_adm->gender=(strcmp(ind_creds[2],"M")==0)?1:0;
		cred_cust->gender=(strcmp(ind_creds[2],"M")==0)?1:0;

		//Filling the Password.
		strcpy(cred_adm->password,ind_creds[3]);
		strcpy(cred_cust->password,ind_creds[3]);

		//Filling the email.
		strcpy(cred_adm->email,ind_creds[4]);
		strcpy(cred_cust->email,ind_creds[4]);

		//Filling the Date of Birth.
		strcpy(cred_adm->DOB,ind_creds[5]);
		strcpy(cred_cust->DOB,ind_creds[5]);

		//Filling the phone.
		strcpy(cred_adm->phone,ind_creds[6]);
		strcpy(cred_cust->phone,ind_creds[6]);

		//Filling the admin key.
		if(entity_type==1)
		{
			strcpy(cred_adm->adminKey,ind_creds[7]);
		}
	}
	else
	{
		//Filling the second credential of login - password.
		strcpy(cred_adm->password,ind_creds[1]);
		strcpy(cred_cust->password,ind_creds[1]);

		//Filling the third credentials of login - adminkey(exclusive to admin login).
		if(entity_type==1)
		{
			strcpy(cred_adm->adminKey,ind_creds[2]);
		}
	}
}

/*
//Temporary function used to fill the fields of customer,admin
void fill_fields(void *entity,char *information,int is_admin)
{
	char entity_information[BUFFER_SIZE];
	strcpy(entity_information,information);
	//printf("inside fill_fields\n");

	//Iteratively filling in the information into the struct.
	//Filling the name.
	char *ind_info = strtok(entity_information, "?");
	strcpy(((customer *)entity)->name, ind_info);
	//printf("error after name: %s\n",((customer *)entity)->name);

	//Filling the age.
	ind_info = strtok(NULL, "?");
	((customer *)entity)->age = atoi(ind_info);
	//printf("error after age.\n");

	//Filling the gender.
	ind_info = strtok(NULL, "?");
	((customer *)entity)->gender = (strcmp(ind_info, "M") == 0) ? 1 : 0;
	//printf("error after gender\n");

	//Filling the password.
	ind_info = strtok(NULL, "?");
	strcpy(((customer *)entity)->password, ind_info);
	//printf("error after pass\n");

	//Filling the email.
	ind_info = strtok(NULL, "?");
	strcpy(((customer *)entity)->email, ind_info);
	//printf("error after email\n");

	//Filling the Date of birth.
	ind_info = strtok(NULL, "?");
	strcpy(((customer *)entity)->DOB, ind_info);
	//printf("(customer*)entity->DOB: %s\n",((customer*)entity)->DOB);

	//Filling the phone.
	ind_info = strtok(NULL, "?");
	strcpy(((customer *)entity)->phone, ind_info);
	//printf("(customer*)entity->phone: %s\n",((customer*)entity)->phone);

	//Filling the admin key only if the entity is an admin.
	if(is_admin)
	{
		ind_info=strtok(NULL,"?");
		strcpy(((admin*)entity)->adminKey,ind_info);
		//printf("error after admin key\n");
	}
}

//Function to fill the given customer entity with the information.
void fill_customer(customer *cust,char *information)
{
	fill_fields(cust,information,0);
}

//Function to fill the given admin entity with the information.
void fill_admin(admin *adm,char *information)
{
	fill_fields(adm,information,1);
}
*/

//Function to extract the book information from the book struct and place it inside the given char buffer.
void extract_book_info(char *buff,book *book)
{
        if(buff==NULL)
        {
                perror("extract_book_info(), given buffer is NULL: ");
                exit(1);
        }
        snprintf(buff,BUFFER_SIZE,"%-30s %-30s %13lld %3d %5d",book->title,book->author,book->ISBN,book->quantity,book->price);
}

//Function to print the columns while displaying library(title,author,isbn,price,quantity)
void get_library_columns(char *buff)
{
        snprintf(buff,BUFFER_SIZE,"%-30s %-30s %-15s %-10s %-15s","TITLE","AUTHOR","ISBN","QUANTITY","PRICE(rupees)");
}

//Function to return the index of the array(books_borrowed) where the read_book has been borrowed by the customer. Returns the macro 'CUSTOMER_DOES_NOT_HAVE_THE_BOOK' if the book hasn't been borrowed.
int customer_has_book(customer *cust,book *read_book)
{
	int count=cust->books_borrowed_count;
	printf("count in customer_has_book: %d\n",count);
	for(int i=0;i<count;i++)
	{
		printf("in customer_has_book loop, ISBN being sched for : %lld\n",(cust->books_borrowed[i]).borrowedbook.ISBN);
		printf("read_book.ISBN: %lld\n",read_book->ISBN);
		if((cust->books_borrowed[i]).borrowedbook.ISBN==read_book->ISBN)
		{
			return i;
		}
	}

	return -1;
}

void convert_time_t_to_string(char *buff,time_t timestamp)
{
	struct tm *t=localtime(&timestamp);
	strftime(buff,BUFFER_SIZE,"%Y-%m-%d %H:%M:%S",t);
}

void append_borrow_return_date(char *buff,bbook borrow_book)
{
	char MSG[BUFFER_SIZE];
	extract_book_info(MSG,&borrow_book.borrowedbook);

	char TIME_MSG[BUFFER_SIZE];
	convert_time_t_to_string(TIME_MSG,borrow_book.borrow_date);
	strcat(MSG,"\n");
	strcat(MSG,TIME_MSG);

	convert_time_t_to_string(TIME_MSG,borrow_book.return_date);
	strcat(MSG,"\n");
	strcat(MSG,TIME_MSG);

	strcpy(buff,MSG);
}
