#include"../include/logging.h"

int write_transaction(char *entity_name,int entity_type,char *activity,bbook *transacted_book,int ret_val,FILE *log_ptr,struct flock *lock,int conn_sock)
{
	transaction *log=(transaction*)malloc(sizeof(transaction));
	MALLOC_FAIL_CHECK(log,"write_transaction()","transaction")
	if(log_ptr==NULL)
	{
		perror("Log file pointer is null: ");
		return LOG_ERROR;
	}

	//Filling the log information into the struct.
	printf("start filling the log\n");
	if(entity_name!=NULL)
	{
		strcpy(log->entity_name,entity_name);
	}
	log->entity_type=entity_type;
	strcpy(log->activity_performed,activity);
	if(transacted_book!=NULL)
	{
		log->transaction_book=*transacted_book;
	}
	log->transaction_state=ret_val;
	printf("done filling the log\n");

	acquire_write_lock(log_ptr,lock,conn_sock);
	int mv_lp_end=fseek(log_ptr,0,SEEK_END);
	if(mv_lp_end!=0)
	{
		perror("fseek(): ");
		return SYS_ERROR;
	}
	int write_stat=fwrite(log,sizeof(transaction),1,log_ptr);
	release_acquired_write_lock(log_ptr,lock,conn_sock);
	if(write_stat!=1)
	{
		return SYS_ERROR;
	}
	printf("done writing the log successfully\n");

	return LOG_SUCCESS;
}

void add_log(char *entity_name,int entity_type,char *activity,bbook *transacted_book,int ret_val,FILE *log_ptr,struct flock *lock,int conn_sock)
{
	int write_stat=write_transaction(entity_name,entity_type,activity,transacted_book,ret_val,log_ptr,lock,conn_sock);
	printf("done writing transaction\n");
	if(write_stat==LOG_SUCCESS)
	{
		printf("Log was successfully written to the LOG File.\n");
	}
	else
	{
		printf("ERROR Adding the Log.\n");
	}
}
