#include<stdio.h>
#include<sqlite3.h>
#include "chat_application_clients.h"

int initialize_database(sqlite3 **db)
{
  char *errMsg = NULL;
  //Create a file to hold data 
  if( sqlite3_open("chat_history.db",db) != SQLITE_OK )
  {
    printf("Failed to open Database\n");
    return FAILURE;
  }
  
  //Create SQL table
  char *history_table = "CREATE TABLE IF NOT EXISTS ChatHistory ("
                        "sender TEXT,"
                        "message TEXT,"
                        "receiver TEXT"
                        ");";
                        
  //Execute this command
  if(sqlite3_exec(*db,history_table,0,0,&errMsg) != SQLITE_OK)
  {
    printf("Failed to create table because %s\n",errMsg);
    sqlite3_free(errMsg);
    return FAILURE;
  }
  
  //Return the database pointer
  return FAILURE;
}
