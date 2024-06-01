#include"../include/homepage.h"

void get_page_options(char *buff,char *option)
{
	IS_BUFF_OR_OPTION_NULL(buff,option,"get_page_options()")
	COPY_BASED_ON_OPTION_INTO_BUFFER(buff,option,LOGIN_OPTIONS,SIGNUP_OPTIONS)
}
void get_required_credentials(char *buff,char *main_option,char *inner_option)
{
	IS_BUFF_OR_OPTION_NULL(buff,main_option,"get_required_credentials()");
	IS_BUFF_OR_OPTION_NULL(buff,inner_option,"get_required_credentials()");
	if(strcmp(main_option,"1")==0)
	{
		required_login_credentials(buff,inner_option);
	}
	else if(strcmp(main_option,"2")==0)
	{
		required_signup_credentials(buff,inner_option);
	}
}
void required_login_credentials(char *buff,char *option)
{
	IS_BUFF_OR_OPTION_NULL(buff,option,"required_login_credentials()")
	COPY_BASED_ON_OPTION_INTO_BUFFER(buff,option,ADMIN_LOGIN_CREDENTIALS_MSG,CUSTOMER_LOGIN_CREDENTIALS_MSG)
}
void required_signup_credentials(char *buff,char *option)
{
	IS_BUFF_OR_OPTION_NULL(buff,option,"required_signup_credentials()")
	COPY_BASED_ON_OPTION_INTO_BUFFER(buff,option,ADMIN_SIGNUP_CREDENTIALS_MSG,CUSTOMER_SIGNUP_CREDENTIALS_MSG)
}

void choose_validate_calls(char *validate_calls,char *main_option,char *inner_option)
{
	if(strcmp(main_option,"1")==0)
	{
		if(strcmp(inner_option,"1")==0)
		{
			strcpy(validate_calls,VALIDATE_ADMIN_LOGIN_CREDENTIALS);
		}
		else if(strcmp(inner_option,"2")==0)
		{
			strcpy(validate_calls,VALIDATE_CUSTOMER_LOGIN_CREDENTIALS);
		}
	}
	else if(strcmp(main_option,"2")==0)
	{
		if(strcmp(inner_option,"1")==0)
		{
			strcpy(validate_calls,VALIDATE_ADMIN_SIGNUP_CREDENTIALS);
		}
		else if(strcmp(inner_option,"2")==0)
		{
			strcpy(validate_calls,VALIDATE_CUSTOMER_SIGNUP_CREDENTIALS);
		}
	}
}
