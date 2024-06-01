#include"../include/locking.h"

int write_lock(FILE *ptr,struct flock *lock)
{
	if(ptr==NULL)
	{
		return LOCK_ERROR;
	}

	memset(lock,0,sizeof(struct flock));
	
	lock->l_type=F_WRLCK;
	lock->l_whence=SEEK_SET;
	lock->l_start=0;
	lock->l_len=0;

	if(fcntl(fileno(ptr),F_SETLK,lock)==-1)
	{
		perror("server write lock fcntl(): ");
		return LOCK_ERROR;
	}

	return LOCK_SUCCESS;
}

int read_lock(FILE *ptr,struct flock *lock)
{
	if(ptr==NULL)
	{
		return LOCK_ERROR;
	}

	memset(lock,0,sizeof(struct flock));
	
	lock->l_type=F_RDLCK;
	lock->l_whence=SEEK_SET;
	lock->l_start=0;
	lock->l_len=0;

	if(fcntl(fileno(ptr),F_SETLK,lock)==-1)
	{
		perror("server read lock fcntl(): ");
		return LOCK_ERROR;
	}

	return LOCK_SUCCESS;
}

int release_write_lock(FILE *ptr,struct flock *lock)
{
	if(ptr==NULL)
	{
		return LOCK_RELEASE_ERROR;
	}

	memset(lock,0,sizeof(struct flock));
	
	lock->l_type=F_UNLCK;
	lock->l_whence=SEEK_SET;
	lock->l_start=0;
	lock->l_len=0;

	if(fcntl(fileno(ptr),F_SETLK,lock)==-1)
	{
		perror("server write release lock fcntl(): ");
		return LOCK_RELEASE_ERROR;
	}

	return LOCK_RELEASE_SUCCESS;
}

int release_read_lock(FILE *ptr,struct flock *lock)
{
	if(ptr==NULL)
	{
		return LOCK_RELEASE_ERROR;
	}

	memset(lock,0,sizeof(struct flock));
	lock->l_type=F_UNLCK;
	lock->l_whence=SEEK_SET;
	lock->l_start=0;
	lock->l_len=0;

	if(fcntl(fileno(ptr),F_SETLK,lock)==-1)
	{
		perror("server read release lock fcntl(): ");
		return LOCK_RELEASE_ERROR;
	}

	return LOCK_RELEASE_SUCCESS;
}

//Functions to acquire locks and release them.
void acquire_write_lock(FILE *ptr,struct flock *lock,int conn_sock)
{
	int lock_status=write_lock(ptr,lock);
	if(lock_status==LOCK_ERROR)
	{
		send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
		send(conn_sock,"FAILED TO ACQUIRE WRITE LOCK",BUFFER_SIZE,0);
	}
}

void acquire_read_lock(FILE *ptr,struct flock *lock,int conn_sock)
{
	int lock_status=read_lock(ptr,lock);
	if(lock_status==LOCK_ERROR)
	{
		send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
		send(conn_sock,"FAILED TO ACQUIRE READ LOCK",BUFFER_SIZE,0);
	}
}

void release_acquired_write_lock(FILE *ptr,struct flock *lock,int conn_sock)
{
	int release_lock_status=release_write_lock(ptr,lock);
	if(release_lock_status==LOCK_RELEASE_ERROR)
	{
		send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
		send(conn_sock,"FAILED TO RELEASE WRITE LOCK",BUFFER_SIZE,0);
	}
}

void release_acquired_read_lock(FILE *ptr,struct flock *lock,int conn_sock)
{
	int release_lock_status=release_read_lock(ptr,lock);
	if(release_lock_status==LOCK_RELEASE_ERROR)
	{
		send(conn_sock,ONLY_PRINT_MSG,OPTION_SIZE,0);
		send(conn_sock,"FAILED TO RELEASE READ LOCK",BUFFER_SIZE,0);
	}
}
