#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include"common_utils.h"
#include"login.h"
#include"signup.h"

//Macros
#define HOMEPAGE_OPTIONS "1 - Login\n2 - Sign up"
#define IS_BUFF_OR_OPTION_NULL(BUFF,OPTION,FUNCTION_NAME) if(BUFF==NULL){printf("in %s, buff is null\n",FUNCTION_NAME);}else if(OPTION==NULL){printf("in %s, option is null\n",FUNCTION_NAME);}
#define COPY_BASED_ON_OPTION_INTO_BUFFER(BUFF,OPTION,OPTION1,OPTION2) if(strcmp(OPTION,"1")==0){strcpy(BUFF,OPTION1);}else if(strcmp(OPTION,"2")==0){strcpy(BUFF,OPTION2);}

void get_page_options(char *buff,char *option);						//Function to get the different options of login,signup.
void get_required_credentials(char *buff,char *main_option,char *inner_option);		//Function to choose between the below 2 functions.
void required_login_credentials(char *buff,char *option);		//Function to get the required credential fields based on who's logging in.
void required_signup_credentials(char *buff,char *option);		//Function to get the required credential fields based on who's signing up.
void choose_validate_calls(char *validate_calls,char *main_option,char *inner_option);	//Function to choose the Validate Functions to be called.
#endif
