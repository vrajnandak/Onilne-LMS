#ifndef LOGIN_H
#define LOGIN_H

#include"common_utils.h"

//Macros
#define LOGIN_OPTIONS 			"Choose one of the following options\n1 - Login as Admin\n2 - Login as Customer"
#define CUSTOMER_LOGIN_CREDENTIALS_MSG	"START SCAN?Please Enter the Following details to Login\nEnter User Name?Enter Password?END SCAN"
#define ADMIN_LOGIN_CREDENTIALS_MSG	"START SCAN?Please Enter the Following details to Login\nEnter Admin Name?Enter Password?Enter Admin key?END SCAN"
#define VALIDATE_CUSTOMER_LOGIN_CREDENTIALS	"10010000"		//Macro to decide the validate functions to be called on login by customer.
#define VALIDATE_ADMIN_LOGIN_CREDENTIALS 	"10010001"		//Macro to decide the validate functions to be called on login by admin.
#define LOGIN_SUCCESS "Login status: Successfull."					//Macro for successfull Login.
#define LOGIN_FAILURE "Login status: Failed to login. Please enter 'r' to retry"	//Macro for failure during Login.

#endif
