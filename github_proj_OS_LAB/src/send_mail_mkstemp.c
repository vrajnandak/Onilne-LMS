#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
int main()
{
	char *user_email=malloc(100*(sizeof(char)));
	printf("Enter your email to which you want the mail to be sent\n");
	scanf("%s",user_email);
	srand(time(0));              
	int otp_int=0;
	for(int i=0;i<6;i++) 
		otp_int=otp_int*10+(rand()%10+1);
	char otp_str[500];
	sprintf(otp_str,"%d",otp_int); 
	char mail[500]="\nThis is your OTP. Please don't share it with anyone.\nIf you are not the intended recipient, please contact XXXXXXXX23 for more information.\nPlease do get your account blocked in our Library or contact our custom Developers.\nWe sincerely apologise for any inconvenience cause .//..#.\n";
	strcat(otp_str,mail); 

	char command[500];                          //holds the command.
	char tempfile[200];            
	strcpy(tempfile,"./sendmailXXXXXX");     //template for temporary file.Since write permissions are required for this code to work, the program must be run in a directory with write permissions such as the current working directory.
	int fd= mkstemp(tempfile);       //generates the tempfile and opens it for writing.
	FILE *fp=fdopen(fd,"w");         //opens file stream for writing.
	fprintf(fp,"%s\n",otp_str);      //writes the body to the file.
	fclose(fp);                      //closes the file stream.
	snprintf(command,sizeof(command),"sendmail %s < %s",user_email,tempfile);  //prepares the command.
	system(command);                      //executes the command.
	remove(tempfile);                //deletes the temporary file that otherwise would be created in the current working directory.
	printf("Enter the otp\n");
	int user_otp;
	scanf("%d",&user_otp);
	if(user_otp==otp_int)
		printf("You have entered the correct otp\n");
	else
		printf("You have entered the wrong otp\nThe correct otp was %d but you entered %d\n",otp_int,user_otp);
	return 0;
}
