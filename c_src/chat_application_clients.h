#ifndef HEADER_NAME
#define HEADER_NAME

#include<sqlite3.h>
int initialize_database(sqlite3 **db);
int store_data_in_database( sqlite3 *db, char *username, char *table_type );
int read_data_from_database( sqlite3 *db, char ***username, int *capacity, char *table_type );
int delete_data_from_friend_names_table(sqlite3 *db, char *username);

#define FAILURE -1
#define SUCCESS 0
#define USERNAME_LENGTH 5
#define MINIMUM_PASSWORD_LENGTH 8
#define MAXIMUM_PASSWORD_LENGTH 15

extern int socket_fd;
extern char logged_in_user[6];
extern sqlite3 *database;

#endif
