#include<stdio.h>
#include <string.h>
#include <stdlib.h>
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
  
  //Create SQL table to store conversation history
  char *history_table = "CREATE TABLE IF NOT EXISTS ChatHistory ("
                        "sender TEXT,"
                        "message TEXT,"
                        "receiver TEXT"
                        ");";

  //Store Friend Name
  char *friend_name = "CREATE TABLE IF NOT EXISTS FriendNames ("
                        "friend_name TEXT UNIQUE"
                        ");";
                        
  //Execute this command
  if(sqlite3_exec(*db,history_table,0,0,&errMsg) != SQLITE_OK)
  {
    printf("Failed to create ChatHistory table because %s\n",errMsg);
    sqlite3_free(errMsg);
    return FAILURE;
  }
  errMsg = NULL;
  //Execute this command
  if(sqlite3_exec(*db,friend_name,0,0,&errMsg) != SQLITE_OK)
  {
    printf("Failed to create FriendNames table because %s\n",errMsg);
    sqlite3_free(errMsg);
    return FAILURE;
  }
  
  //Return the database pointer
  return SUCCESS;
}

int store_data_in_database( sqlite3 *db, char *username, char *table_type )
{  
  sqlite3_stmt *stmt;
  int rc = 0;
  //printf("FN %s\n",username);
  const char *friend_name = NULL;
  if( strcmp(table_type,"friendnames") == 0 )
  {
    friend_name = "INSERT OR IGNORE INTO FriendNames(friend_name)"
                      "VALUES(?);";

    rc = sqlite3_prepare_v2(db, friend_name, -1, &stmt, NULL);


    if (rc != SQLITE_OK)
    {
        printf("Prepare failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

  }
  

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE)
  {
      printf("Insert failed: %s\n", sqlite3_errmsg(db));
      sqlite3_finalize(stmt);
      return 1;
  }

  sqlite3_finalize(stmt);                       


  return SUCCESS;
}
int read_data_from_database( sqlite3 *db, char ***username, int *capacity, char *table_type )
{
//printf("Here in read \n");
    sqlite3_stmt *stmt;
    int rc = 0, count = 0;
    const char *friend_name = NULL;
    if( strcmp(table_type,"friendnames") == 0 )
    {

      friend_name = "SELECT * FROM FriendNames";

      if(sqlite3_prepare_v2(db, friend_name, -1, &stmt, NULL) != SQLITE_OK)
      {
          printf("Prepare failed: %s\n", sqlite3_errmsg(db));
          return 1;
      }  

    }//printf("Here in read1 \n");
    const char *name = "";

    while( ( rc = sqlite3_step(stmt) ) == SQLITE_ROW )
    {
//printf("Here in read2 \n");
        name = (const char *)sqlite3_column_text(stmt,0);

        if(name == NULL)
        {
          name = "";
          return 0;
        }

        if(count == *capacity)
        {
            *capacity *= 2;

            char **tmp = realloc(*username, (*capacity) * sizeof(char*));

            if(tmp == NULL)
                return FAILURE;

            *username = tmp;
        }

        (*username)[count] = strdup(name);

        count++;
    }
//printf("CP-1 %d\n",*capacity);
    if(rc != SQLITE_DONE)
    {
      printf("Execution error: %s\n", sqlite3_errmsg(db));
      return FAILURE;
    }
    else if(rc == SQLITE_DONE && count == 0)
    {
          sqlite3_finalize(stmt);

      return count;
    }

    return count;
}

int delete_data_from_friend_names_table(sqlite3 *db, char *username)
{
    sqlite3_stmt *stmt;
    int rc = 0;
    
    // 1. The SQL Query with a placeholder (?)
    const char *sql = "DELETE FROM FriendNames WHERE friend_name = ?;";

    // 2. Prepare the statement
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("Prepare failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // 3. Bind the username to the ? placeholder
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    // 4. Execute the statement
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        printf("Delete execution failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }
    // 5. Finalize/Clean up
    sqlite3_finalize(stmt);
    return 0; // Success
}
//Logic should be written to close the database
