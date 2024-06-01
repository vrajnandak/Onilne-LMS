#ifndef CLIENT_H
#define CLIENT_H

#include"common_utils.h"
#include"Buffer_utils.h"

#define SRV_ADDRESS "127.0.0.1"			//Defining macro variables for server's values of port number, address.

void scan_all_inputs_and_send(int fsock);	//Function that take input from User after printing a message parsed from the given buffer.

#endif
