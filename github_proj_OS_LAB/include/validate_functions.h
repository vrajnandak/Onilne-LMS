#ifndef VALIDATE_FUNCTIONS_H
#define VALIDATE_FUNCTIONS_H

#include"common_utils.h"
#include"structures.h"				//To get the length of the name,email,address strings etc.

//Macros
#define VALIDATE_STRING_LENGTH 8

#define VALID_CREDENTIALS 0
#define INVALID_CREDENTIALS 1

#define VALID_NAME 0
#define INVALID_NAME 1

#define VALID_AGE 0
#define INVALID_AGE 1

#define VALID_GENDER 0
#define INVALID_GENDER 1

#define VALID_PASSWORD 0
#define INVALID_PASSWORD 1

#define VALID_EMAIL 0
#define INVALID_EMAIL 1

#define VALID_DATE 0
#define INVALID_DATE 1

#define VALID_PHONE 0
#define INVALID_PHONE 1

#define VALID_ADMINKEY 0
#define INVALID_ADMINKEY 1

//Macro to return appropriate value based on the return value of the validate function called.
#define CHECK_RET_VAL(RET_VAL,ERROR_STATUS_CODE) if(RET_VAL==ERROR_STATUS_CODE){return INVALID_CREDENTIALS;}else if(RET_VAL==POINTER_IS_NULL){return POINTER_IS_NULL;}

//Intermediate functions.
int is_string_number(char *buff,int size);	//Returns 0 if the string is a number else returns 1.
int is_leap_year(int year);			//Checks whether the given year is a leap year or not. Returns 1 if it is a leap year.
int is_valid_date(char *date);			//Checks if the given Date is a valid date in the calendar.
int is_in_adminkey_form(char *adminkey);	//Checks if the given adminkey is of form 'AdminKeyX' where 'X' is either of 1 or 2 or 3.

//Validate functions.
int is_name_valid(char *name);			//Checks to see if the name is a valid string of length specified in structures.h.
int is_age_valid(char *age);			//Checks to see if the age is a valid number.
int is_gender_valid(char *gender);		//Checks to see if the gender is within the specified options.
int is_password_valid(char *pass);		//Checks to see if the password is a valid string of length specified in structures.h.
int is_email_valid(char *email);		//Checks to see if the email is a valid email by sending an otp to the client email specified.
int is_DOB_valid(char *DOB);			//Checks to see if the Date of Birth is a valid date of birth.
int is_phone_valid(char *phone);		//Checks to see if the phone is a valid number (simply checks if it is a 10 digit number).
int is_admin_key_valid(char *adminkey);		//Checks to see if the admin key is a valid key (has to be of form 'adminkeyX' where 'X' is one of 1,2,3.

//Function to choose the validate functions to call based on the binary bits in the 'validate_calls' string. The bits correspond to the Validate functions in the same order as they are specified above.
int validate_creds(char *information,char *validate_calls);

#endif
