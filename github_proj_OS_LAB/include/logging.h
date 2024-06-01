#ifndef LOGGING_H
#define LOGGING_H

#include"common_utils.h"
#include"structures.h"
#include"locking.h"

#define LOG_SUCCESS 0
#define LOG_ERROR 1
#define SYS_ERROR 2

//Function to write the transaction to the log file.
int write_transaction(char *entity_name,int entity_type,char *activity,bbook *transacted_book,int ret_val,FILE *log_ptr,struct flock *lock,int conn_sock);
void add_log(char *entity_name,int entity_type,char *activity,bbook *transacted_book,int ret_val,FILE *log_ptr,struct flock *lock,int conn_sock);

#endif
