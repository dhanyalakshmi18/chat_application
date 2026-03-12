#ifndef HEADER_NAME
#define HEADER_NAME

#include<sqlite3.h>

#define FAILURE -1
#define SUCCESS 0
#define USERNAME_LENGTH 5
#define MINIMUM_PASSWORD_LENGTH 8
#define MAXIMUM_PASSWORD_LENGTH 15

extern int socket_fd;
extern char logged_in_user[6];
extern sqlite3 *database;

#endif
