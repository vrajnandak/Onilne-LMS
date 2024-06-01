#ifndef LOCKING_H
#define LOCKING_H

#include<fcntl.h>
#include"common_utils.h"

#define LOCK_SUCCESS 0
#define LOCK_ERROR 1

#define LOCK_RELEASE_SUCCESS 0
#define LOCK_RELEASE_ERROR 1

//Functions implementing the write, read locks and releasing the locks.
int write_lock(FILE *ptr,struct flock *lock);
int read_lock(FILE *ptr,struct flock *lock);

int release_write_lock(FILE *ptr,struct flock *lock);
int release_read_lock(FILE *ptr,struct flock *lock);


//Functions to acquire locks and release them.
void acquire_write_lock(FILE *ptr,struct flock *lock,int conn_sock);
void acquire_read_lock(FILE *ptr,struct flock *lock,int conn_sock);
void release_acquired_write_lock(FILE *ptr,struct flock *lock,int conn_sock);
void release_acquired_read_lock(FILE *ptr,struct flock *lock,int conn_sock);
#endif
