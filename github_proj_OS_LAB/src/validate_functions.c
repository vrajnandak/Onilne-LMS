#include"../include/validate_functions.h"

//Intermediate functions.
int is_string_integer(char *buff,int size)
{
	IS_POINTER_NULL(buff)
	for(int i=0;i<size;i++)
	{
		if(!isdigit(buff[i]))
		{
			printf("not a digit: %c\n",buff[i]);
			return 0;
		}
	}

	return 1;
}
int is_leap_year(int year)
{
	return (year%4==0 && year%100!=0) || (year%400 ==0);
}
int is_valid_date(char *date)
{
	IS_POINTER_NULL(date)
	int year,month,day;

	char tmp_date[DATE_LENGTH];					//Using a temporary buffer to ensure that 'date' is not freed or erased.
	strcpy(tmp_date,date);
	if(sscanf(tmp_date,"%d-%d-%d",&year,&month,&day)!=3)		//Checking if the date is in correct format.
	{
		perror("in is_valid_date(), sscanf(): ");
		return INVALID_DATE;
	}

	//Check if the year,month,date are in valid range.
	if(year<0 || month<1 || month>12 || day<1)
	{
		printf("year or month or date are not in valid range\n");
		return INVALID_DATE;
	}

	int days_in_month[]={31,28,31,30,31,30,31,31,30,31,30,31};	//Setting the days in every month to match that of the years.
	if(is_leap_year(year))
	{
		days_in_month[1]=29;
	}

	if(day>days_in_month[month-1])					//Checking if the day lies within the number of days in the month.
	{
		printf("given day is greater than the number of days in the month\n");
		return INVALID_DATE;
	}

	return VALID_DATE;
}
int is_in_adminkey_form(char *adminkey)
{
	IS_POINTER_NULL(adminkey)
	
	char tmp_adminkey[ADMINKEY_LENGTH];				//Temporary buffer to ensure that 'adminkey' is not freed or erased.
	strcpy(tmp_adminkey,adminkey);

	printf("tmp_adminKey: %s\n",tmp_adminkey);
	int access_level;
	if(sscanf(tmp_adminkey,ADMINKEY_FORMAT,&access_level)!=1)
	{
		perror("in is_in_adminkey_form(), sscanf(): ");
		return INVALID_ADMINKEY;
	}
	else if(access_level!=1 && access_level!=2 && access_level!=3)
	{
		printf("access level doesn't match\n");
		return INVALID_ADMINKEY;
	}

	return VALID_ADMINKEY;
}

//Validate functions.
int is_name_valid(char *name)
{
	IS_POINTER_NULL(name)
	if(strlen(name)>NAME_LENGTH)
	{
		return INVALID_NAME;
	}
	return VALID_NAME;
}
int is_age_valid(char *age)
{
	IS_POINTER_NULL(age)
	if(!is_string_integer(age,strlen(age)))
	{
		return INVALID_AGE;
	}
	return VALID_AGE;
}
int is_gender_valid(char *gender)
{
	IS_POINTER_NULL(gender)
	if(strcmp(gender,"M") && strcmp(gender,"F"))
	{
		return INVALID_GENDER;
	}
	return VALID_GENDER;
}
int is_password_valid(char *pass)
{
	IS_POINTER_NULL(pass)
	if(strlen(pass)>PASSWORD_LENGTH)
	{
		return INVALID_PASSWORD;
	}

	return VALID_PASSWORD;
}
int is_email_valid(char *email)
{
	IS_POINTER_NULL(email)
	if(strlen(email)>EMAIL_LENGTH)
	{
		return INVALID_EMAIL;
	}
	return VALID_EMAIL;
}
int is_DOB_valid(char *DOB)
{
	IS_POINTER_NULL(DOB)
	if(is_valid_date(DOB)==INVALID_DATE)
	{
		return INVALID_DATE;
	}
	return VALID_DATE;
}
int is_phone_valid(char *phone)
{
	IS_POINTER_NULL(phone)
	if(strlen(phone)>PHONE_LENGTH ||(!is_string_integer(phone,strlen(phone))))
	{
		return INVALID_PHONE;
	}
	return VALID_PHONE;
}
int is_admin_key_valid(char *adminkey)
{
	IS_POINTER_NULL(adminkey)
	if(strlen(adminkey)>ADMINKEY_LENGTH || is_in_adminkey_form(adminkey)==INVALID_ADMINKEY)
	{
		return INVALID_ADMINKEY;
	}
	return VALID_ADMINKEY;
}


//Function to choose the validate functions to call based on the binary bits in the 'validate_calls' string. The bits correspond to the Validate functions in the same order as they are specified above.
int validate_creds(char *information,char *validate_calls)
{
	if(information==NULL || validate_calls==NULL)
	{
		return POINTER_IS_NULL;
	}

	//Copying the credential information into a different buffer.
	char validate_information[BUFFER_SIZE];
	strcpy(validate_information,information);

	//Parsing the information using '?' as the delimiter.
	char *token=strtok(validate_information,"?");
	if(token==NULL)
	{
		return NULL_POINTER_ACCESS_ERROR;
	}

	//use_name will be separately used for calling the validate name function to make parsing easier.
	char use_name[BUFFER_SIZE];
	strcpy(token,use_name);

	int length=strlen(validate_calls);
	//Calling the appropriate Validate functions based on the bits. Function is called if the bit is 1. The association of the bits to the validate functions called is fixed and is as shown in the below 'switch' statement.
	for(int i=0;i<length;i++)
	{
		int bit_val=(validate_calls[i]=='1')?1:0;
		if(bit_val==1 && i)
		{
			token=strtok(NULL,"?");					//Parsing the string.
			if(token==NULL)
			{
				return NULL_POINTER_ACCESS_ERROR;
			}

			int ret_val;
			switch(i)
			{
				case 0: ret_val=is_name_valid(use_name);	//Calling the validate name function.
					CHECK_RET_VAL(ret_val,INVALID_NAME)
					break;
				case 1: ret_val=is_age_valid(token);		//Calling the validate age function.
					CHECK_RET_VAL(ret_val,INVALID_AGE)
					break;
				case 2:	ret_val=is_gender_valid(token);		//Calling the validate gender function.
					CHECK_RET_VAL(ret_val,INVALID_GENDER)
					break;
				case 3:	ret_val=is_password_valid(token);	//Calling the validate password function.
					CHECK_RET_VAL(ret_val,INVALID_PASSWORD)
					break;
				case 4:	ret_val=is_email_valid(token);		//Calling the validate email function.
					CHECK_RET_VAL(ret_val,INVALID_EMAIL)
					break;
				case 5:	ret_val=is_DOB_valid(token);		//Calling the validate date of birth function.
					CHECK_RET_VAL(ret_val,INVALID_DATE)
					break;
				case 6:	ret_val=is_phone_valid(token);		//Calling the validate phone function.
					CHECK_RET_VAL(ret_val,INVALID_PHONE)
					break;
				case 7:	ret_val=is_admin_key_valid(token);	//Calling the validate adminkey function.
					CHECK_RET_VAL(ret_val,INVALID_ADMINKEY)
					break;
			}
		}
	}

	return VALID_CREDENTIALS;
}
