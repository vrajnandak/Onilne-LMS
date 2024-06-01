#ifndef SIGNUP_H
#define SIGNUP_H

#include"common_utils.h"

//Macros
#define SIGNUP_OPTIONS "1 - Signup as Admin\n2 - Signup as Customer"
#define CUSTOMER_SIGNUP_CREDENTIALS_MSG "START SCAN?Please Enter the Following details to Signup\nEnter User Name?Enter your age?Enter your gender(M/F)?Enter Password?Enter Email?Enter DOB in YYYY-MM-DD format?Enter Phone?END SCAN"
#define ADMIN_SIGNUP_CREDENTIALS_MSG "START SCAN?Please Enter the Following details to Signup\nEnter Admin Name?Enter your age?Enter your gender(M/F)?Enter Password?Enter Email?Enter DOB in YYYY-MM-DD format?Enter Phone?Enter Admin key?END SCAN"
#define VALIDATE_CUSTOMER_SIGNUP_CREDENTIALS "11111110"		//Macro to decide the validate functions to be called on signup by Customer.
#define VALIDATE_ADMIN_SIGNUP_CREDENTIALS "11111111"		//Macro to decide the validate functions to be called on signup by Admin.
#define SIGNUP_SUCCESS "Signup status: Successfull."						//Macro for successfull Signup.
#define SIGNUP_FAILURE "Signup status: Failed to Signup. Please enter 'r' to retry"		//Macro for failure during Signup.

#endif
