#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <ctype.h>
#include <sys/socket.h>
#include "chat_application.pb-c.h"

#define FAILURE -1
#define SUCCESS 0
#define USERNAME_LENGTH 5
#define MINIMUM_PASSWORD_LENGTH 8
#define MAXIMUM_PASSWORD_LENGTH 15

int socket_fd = 0;
char logged_in_user[6];

int authentication_details( WINDOW *menu, int authentication_type, char *username, char *password, char *authentication_request_type, int max_user_name_len, int max_passwd_len, int max_auth_req_type_len ) 
{
  werase(menu);
  box(menu, '|', '=');
  echo();
  
  if(authentication_type == 1)
  {
    mvwprintw(menu,1,35,"Register ");
    snprintf(authentication_request_type,max_auth_req_type_len,"register");
  }
  else if( authentication_type == 2 )
  {
    mvwprintw(menu,1,35,"Login ");
    snprintf(authentication_request_type,max_auth_req_type_len,"login");
  }
  else if( authentication_type == 3)
  {
    mvwprintw(menu,1,35,"Deregister ");
    snprintf(authentication_request_type,max_auth_req_type_len,"unregister"); 
  }
  else
  {
    mvwprintw(menu,1,35,"Logout ");
    snprintf(authentication_request_type,max_auth_req_type_len,"logout");
  }
 // echo();
  mvwprintw(menu,3,40,"Username: ");
  wmove(menu, 3, 50);
  wrefresh(menu);
  wgetnstr(menu,username,max_user_name_len);
  wrefresh(menu);
  noecho();
  mvwprintw(menu,4,40,"Password: ");
  wmove(menu, 4, 50);
  wrefresh(menu);
  wgetnstr(menu,password,max_passwd_len);
  
  if(strlen(username) == 0)
  {
    authentication_details( menu, authentication_type, username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len );
  }

  return 0;
}

int validate_password(char *password)
{
  int uppercase = 0, lowercase = 0, number = 0, special_character = 0, i = 0;
  
  for( i=0; i<strlen(password); i++ )
  {
    if( isupper( password[i] ) != 0 )
    {
      uppercase = 1;
    }
    else if( islower( password[i] ) != 0 )
    {
      lowercase = 1;
    }
    else if( isdigit( password[i] ) != 0 )
    {
      number = 1;
    }
    else if( password[i] == '@' || password[i] == '&' )
    {
      special_character = 1;
    }
  }
  
  return (uppercase & lowercase & number & special_character);
}

WINDOW *window_configuration( int win_type)
{
  //Get original window(standard screen) size
  int term_h = 0,term_w = 0;
  int win_h = 0, win_w = 0, start_y = 0, start_x = 0;
  getmaxyx(stdscr,term_h,term_w);
  
  //Set custom screen size as required.
  if(win_type == 1)
  {
    win_h = 10,win_w = 80;
    
    //Compute the top-left coordinates, as they are required to define where the window begins on the screen,mostly to keep the window position in the center.
    start_y = (term_h - win_h)/2;
    start_x = (term_w - win_w)/2;
  }
  else if(win_type == 2)
  {
      // Use the full width of the terminal
      win_w = term_w - 6; 
      
      // Height is Total Height minus the 4 rows we save for the bottom
      win_h = term_h - 4; 
      
      // Start at the very top-left
      start_y = 0;
      start_x = 3;
  }
  else if(win_type == 3)
  {
      // Height is the 4 rows we saved
      win_h = 4;
      
      // Width should match the top window
      win_w = term_w - 6;
      
      // Start exactly where the history window finishes (at row term_h - 4)
      start_y = term_h - 4;
      start_x = 3;
  }
    
  WINDOW *menu = newwin(win_h,win_w,start_y,start_x);
  
  return menu;
}

int success_or_error_window(const char *success_or_error_message[], size_t length)
{
  WINDOW *menu = window_configuration(1);
  int width = getmaxx(menu);

  box(menu,'|','=');
  int i=0,j=2;
  for(i=0;i<length;i++)
  {
    mvwprintw(menu,i+j,2,"%.*s", width - 4,success_or_error_message[i]);
    wrefresh(menu);
  }
  wrefresh(menu);
  mvwprintw(menu,6,2,"%.*s", width - 4,"Press Enter to Continue ");
  wrefresh(menu);
  
  //Wait until user presses 'enter'
  int ch;
  while((ch = wgetch(menu)) != '\n' && ch != KEY_ENTER && ch != '\r');
  werase(menu);
  wrefresh(menu);
  delwin(menu);
  touchwin(stdscr);   // mark main screen as changed
  refresh(); // redraw main screen

  return 0;
}

int welcome_page( WINDOW *menu, char *username, char *password, char *authentication_request_type, int max_user_name_len, int max_passwd_len, int max_auth_req_type_len ) 
{
memset(username, 0, max_user_name_len);
  memset(password, 0, max_passwd_len);
  memset(authentication_request_type, 0, max_auth_req_type_len);
touchwin(menu);
wrefresh(menu);
  //Place this strings inside the custom window.
  mvwprintw(menu,1,35,"WELCOME TO THE CHAT APPLICATION");
  mvwprintw(menu,3,40,"1 - Register");
  mvwprintw(menu,4,40,"2 - Unregister");
  mvwprintw(menu,5,40,"3 - Login");
  mvwprintw(menu,6,40,"0 - Quit the Application");

  //Get user input
  int choice = wgetch(menu);

  if(choice == '0')
  {
    delwin(menu);
    endwin();
    exit(0);
  }
  else if( choice == '1')
  {
    authentication_details( menu, 1, username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len );
    if( strlen(username) != USERNAME_LENGTH || strlen(password)<MINIMUM_PASSWORD_LENGTH || strlen(password)>MAXIMUM_PASSWORD_LENGTH || validate_password(password) == 0 )
    {
      //Call Error window
      const char *error_messages[] =
        {
            "Invalid username/password:",
            "1. Username must be 4 characters",
            "2. Password must be between 8 and 15 characters"
        };
      size_t length = sizeof(error_messages) / sizeof(error_messages[0]);
      success_or_error_window(error_messages,length); 
      welcome_page( menu, username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len );
    }
  }
  else if( choice == '2' )
  {
    authentication_details( menu, 3, username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len );
  }
  else if( choice == '3' )
  {
    authentication_details( menu, 2, username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len );
  }
  else
  {
    mvwprintw(menu,8,5,"Please enter 0 or 1 or 2 or 3 only");
    wrefresh(menu);
    return 1;
  }
  wrefresh(menu);
  return 0;
}

int receive_message( void *message, size_t message_size_to_be_received )
{
  int total_bytes = 0;
  while( total_bytes < message_size_to_be_received )
  {
    ssize_t n = recv(socket_fd, (char *)message + total_bytes, message_size_to_be_received - total_bytes, 0);
	if (n < 0)
    {
      return FAILURE;
    }
    total_bytes += n;
  }
  return 0;
}

int send_message( const void *message, size_t message_size )
{
  int total_bytes = 0;
  while( total_bytes < message_size )
  {
  	ssize_t n = send(socket_fd, (char *)message + total_bytes, message_size - total_bytes, 0 );
    if( n < 0 )
    {
      perror("sending failed");
      return FAILURE;
    }
    total_bytes += n;
  }
  return 0;
}

char *exchange_messages(char *message_type, char *guest_username, char *message)
{
  ExchangeMessage exchange_message = EXCHANGE_MESSAGE__INIT;
  exchange_message.message_type = message_type;
  exchange_message.conversation = guest_username;//To validate user
  
  ChatEnvelope message_exchange_packet = CHAT_ENVELOPE__INIT;
  message_exchange_packet.payload_case = 2; //For exchangemsg
  message_exchange_packet.exchangemsg = &exchange_message;
  //Get the length of this packet
  size_t length_of_packet = chat_envelope__get_packed_size(&message_exchange_packet);
  uint8_t *buffer_to_store_formatted_message = malloc(length_of_packet);
  if(buffer_to_store_formatted_message == NULL)
  {
    fprintf(stderr,"Out of memory\n");
    exit(EXIT_FAILURE);
  }
  
  //Pack the message into protobuf format
  chat_envelope__pack(&message_exchange_packet,buffer_to_store_formatted_message);//ready to send message
  uint32_t length_of_message = htonl(length_of_packet);
  
  send_message(&length_of_message,4);
  send_message(buffer_to_store_formatted_message,length_of_packet);
  free(buffer_to_store_formatted_message);
  buffer_to_store_formatted_message = NULL;
  
  
  uint32_t message_length_to_be_received = 0;
  receive_message(&message_length_to_be_received, 4 );
  uint32_t message_length = ntohl(message_length_to_be_received);
  //allocate memory for receiving message
  char *buffer_to_receive_message = malloc(message_length);
  receive_message(buffer_to_receive_message, message_length);
  AuthenticationReply *reply = authentication_reply__unpack(NULL, message_length, (const uint8_t *)buffer_to_receive_message);
  char *reply_message = strdup(reply->text);
  authentication_reply__free_unpacked(reply,NULL);
  free(buffer_to_receive_message);
  
  return reply_message;
}
char* get_dynamic_input(WINDOW *win) {
    int capacity = 128;         // Initial size
    int length = 0;             // Current number of characters
    char *buffer = malloc(capacity);
    int ch;
    // Move cursor to the start of the input area in the bottom window
    mvwprintw(win,1,5,"Message: ");
    wmove(win,1,15);
    wrefresh(win);

    while ((ch = wgetch(win)) != '\n') { // Loop until ENTER is pressed
        
        // 1. Handle Backspace
        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (length > 0) {
                length--;
                // Move cursor back, print a space to erase, move cursor back again
                int cur_y, cur_x;
                getyx(win, cur_y, cur_x);
                mvwaddch(win, cur_y, cur_x - 1, ' ');
                wmove(win, cur_y, cur_x - 1);
            }
        } 
        // 2. Handle Printable Characters
        else if (ch >= 32 && ch <= 126) { 
            // Check if we need more memory
            if (length + 1 >= capacity) {
                capacity *= 2;
                buffer = realloc(buffer, capacity);
            }
            
            buffer[length++] = ch;
            waddch(win, ch); // Echo the character to the screen
        }
        
        wrefresh(win);
    }

    buffer[length] = '\0'; // Null-terminate the string
    return buffer; // Caller is responsible for free()
}
int chat_window(char *guest_username, char *login_status)
{
  WINDOW *top_win = window_configuration(2);
  WINDOW *bottom_win = window_configuration(3);
  box(bottom_win, '|', '=');
  box(top_win,0,0);
  wrefresh(top_win);    // Show the top window and its border
  wrefresh(bottom_win); // Show the bottom window and its border
  mvwprintw(top_win,1,5,"%s",guest_username);
  wrefresh(top_win);    // Show the top window and its border
  mvwprintw(top_win,2,5,"%s",login_status);
  wrefresh(top_win);    // Show the top window and its border
  noecho();
  char *user_input = get_dynamic_input(bottom_win);
  char *message = exchange_messages("check_identity",guest_username, user_input);
  mvwprintw(top_win,3,5,"INFO: %s",message);
  free(user_input);
  free((void *)message);
  return SUCCESS;
}
int friendlist(char *guest_username, int max_guest_user_name_len)
{
    WINDOW *top_win = window_configuration(2);
    WINDOW *bottom_win = window_configuration(3);
    box(bottom_win, '|', '=');
    box(top_win,0,0);
    wrefresh(top_win);    // Show the top window and its border
    wrefresh(bottom_win); // Show the bottom window and its border
    //echo();
    mvwprintw(top_win,1,5,"CHAT APPLICATION");
    wrefresh(top_win);    // Show the top window and its border
    echo();
    mvwprintw(bottom_win,1,5,"Friend Name: ");
    wmove(bottom_win,1,19);
    wgetnstr(bottom_win,guest_username,max_guest_user_name_len);
    if(strcmp(guest_username,"quit") != 0)
    {
      char *message = exchange_messages("check_identity",guest_username, NULL);
      mvwprintw(top_win,3,5,"INFO: %s",message);
      if(strcmp(message,"Unknown User") == 0)
      {
        werase(top_win);
        wrefresh(top_win);
        const char *messages[] = { message };
        success_or_error_window(messages,1);
        friendlist(guest_username, max_guest_user_name_len);
      }
      else
      {
        chat_window( guest_username, message );
      }
      free((void *)message);
    }
    else
    {
      return 1;
    }
    
    werase(top_win);
    werase(bottom_win);
    wrefresh(top_win);
    wrefresh(bottom_win);
    delwin(top_win);
    delwin(bottom_win);
    refresh();
    return 0;
}

int chat_dashboard( WINDOW *menu, char *guest_username, int max_guest_user_name_len, char *password, int max_passwd_len, char *authentication_request_type, int max_auth_req_type_len )
{
  wrefresh(menu);
  //Place this strings inside the custom window.
  mvwprintw(menu,1,35,"CHAT APPLICATION");
  mvwprintw(menu,3,40,"1 - Start Chatting");
  mvwprintw(menu,4,40,"2 - New Messages");
  mvwprintw(menu,5,40,"3 - History");
  mvwprintw(menu,6,40,"4 - Logout");

  //Get user input
  int choice = wgetch(menu);

  if( choice == '1')
  {
    werase(menu);
    wrefresh(menu); //to erase previous window content from the terminal
    /*delwin(menu);
    clear();
    refresh();*/
    friendlist(guest_username,max_guest_user_name_len);
    return 1;
  }
  else if( choice == '4' )
  {
    authentication_details(menu,choice,guest_username,password,authentication_request_type,max_guest_user_name_len,max_passwd_len,max_auth_req_type_len);
    return 2;
  }
  else
  {
    mvwprintw(menu,6,5,"Please enter 1 or 2 or 3 or 4 only");
    return 1;
  }
  return SUCCESS;
}

int create_custom_window( WINDOW *menu, char *page_type, char *username, char *password, char *authentication_request_type, int max_user_name_len, int max_passwd_len, int max_auth_req_type_len)
{ 
	int choice = 0;
  while(1)
  {
    //Create a box around the window with the characters passed as an arguments.
    box(menu,'|','=');
    
    if( strcmp(page_type,"welcome_page") == 0 )
    {
      //To return back to main page
      while( welcome_page( menu, username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len ) == 1)
      {
      }
      break;
    }
    else if( strcmp(page_type, "chat_dashboard") == 0)
    {
      while( (choice = chat_dashboard( menu, username, max_user_name_len, password, max_passwd_len, authentication_request_type, max_auth_req_type_len )) == 1 )
      {
        create_custom_window( menu, page_type, username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len);
      }
      if( choice == 2 )
      {
          return 1;
      }
      break;
    }
    else
    {
      continue;
    }
   }
    
  return SUCCESS;
}

int encode_or_decode_messages( char *username, char *password, char *authentication_request_type, int max_user_name_len, int max_passwd_len, int max_auth_req_type_len) 
{
	//Initialize protobuf and update structure values in that proto
	Authentication auth_req = AUTHENTICATION__INIT;
	auth_req.request_type = authentication_request_type;
        auth_req.username = username;
        auth_req.password = password;
        
        ChatEnvelope chat_packet = CHAT_ENVELOPE__INIT;
        chat_packet.payload_case = 1; // Or CHAT_ENVELOPE__PAYLOAD_AUTH
        chat_packet.auth = &auth_req;
	size_t length_of_packet = chat_envelope__get_packed_size(&chat_packet);
	uint8_t *buffer_to_store_formatted_message = malloc(length_of_packet);
	if(buffer_to_store_formatted_message == NULL) 
	{
		return FAILURE; // Out of memory
	}
	chat_envelope__pack(&chat_packet,buffer_to_store_formatted_message);
	uint32_t length_of_message = htonl(length_of_packet);
	//send message to Erlang process
	send_message(&length_of_message,4);
	send_message(buffer_to_store_formatted_message,length_of_packet);
	free(buffer_to_store_formatted_message);
	buffer_to_store_formatted_message = NULL;
	uint32_t message_length_to_be_received = 0;
	receive_message(&message_length_to_be_received, 4 );
	uint32_t message_length = ntohl(message_length_to_be_received);
	//allocate memory for receiving message
	char *buffer_to_receive_message = malloc(message_length);
	receive_message(buffer_to_receive_message, message_length);
	AuthenticationReply *reply = authentication_reply__unpack(NULL, message_length, (const uint8_t *)buffer_to_receive_message);
	const char *reply_message[] = {reply->text};
	size_t length = sizeof(reply_message) / sizeof(reply_message[0]);
	success_or_error_window(reply_message,length);
        if(strcmp(*reply_message,"Successfully Logged In") == 0)
        {
            strcpy(logged_in_user,username);
            authentication_reply__free_unpacked(reply,NULL);
            free(buffer_to_receive_message);
            return 1;
        }
        else if(strcmp(*reply_message,"Incorrect Username or Password-Failed to LogOut") == 0 )
        {
            authentication_reply__free_unpacked(reply,NULL);
            free(buffer_to_receive_message);
            return 1;
        }
        authentication_reply__free_unpacked(reply,NULL);
        free(buffer_to_receive_message);
	return 0;
}

int initialize_socket() 
{
  //Initialize socket
  int socket_file_descriptor = socket(AF_INET,SOCK_STREAM,0);
  if (socket_file_descriptor == -1)
  {
      perror("socket_creation_failed");
      exit(EXIT_FAILURE);
  }
  
  //Append server information
  struct sockaddr_in socket_address = {0};
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(1234);//server port number
  socket_address.sin_addr.s_addr = inet_addr("192.168.21.67");//server ip address
  //Connect to server
  if( connect(socket_file_descriptor,(struct sockaddr *)&socket_address,sizeof(socket_address)) == FAILURE )
  {
    //Print the reason of the error
    perror("Connection_request_failed");
    close(socket_file_descriptor);
    exit(EXIT_FAILURE);
  }
  printf("Connection established successfully\n");
  
  return socket_file_descriptor;
}

int register_loop( WINDOW *menu, char *authentication_request_type,char *username, char *password, int max_user_name_len, int max_passwd_len, int max_auth_req_type_len )
{
  while(strcmp(authentication_request_type,"register") == 0)
  {
    memset(username, 0, max_user_name_len);
    memset(password, 0, max_passwd_len);
    create_custom_window( menu, "welcome_page", username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len );
    if(encode_or_decode_messages( username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len) == 1)
    {
	  break;
    }
    strcpy(authentication_request_type, "register");
  }
  int choice = 1;
  while( (choice = create_custom_window( menu, "chat_dashboard", username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len )) == 1 )
  {
    //This makes sure that the user who has logged in he can logout himself but cannot logout other users.
      if(strcmp(username,logged_in_user) == 0)
      {
        if( encode_or_decode_messages( username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len) == 1)
        {
          continue;
        }
        else
        {
          strcpy(authentication_request_type, "register");
          return 1;
        }
      }
      else
      {
        const char *msg[] = { "You cannot logout other users" };
        success_or_error_window(msg,1);
      }
  }
  
  //
  return SUCCESS;
}

int main(int argc,char **argv)
{  
  socket_fd = initialize_socket();
  int max_user_name_len = USERNAME_LENGTH;
  char *username = malloc(max_user_name_len+1);
  if (username == NULL) {
    perror("Username malloc failed: ");
    exit(EXIT_FAILURE);
  }
  
  /*int max_guest_username_length = USERNAME_LENGTH;
  char *guest_username = malloc(max_guest_username_length+1);
  if (guest_username == NULL) {
    perror("Guest Username malloc failed: ");
    exit(EXIT_FAILURE);
  }*/
  
  int max_passwd_len = 16;//Since password length is 8 to 15 and +1 for null character.
  char *password = malloc(max_passwd_len+1);
  if (password == NULL) {
    perror("Password malloc failed: ");
    exit(EXIT_FAILURE);
  }
  
  int max_auth_req_type_len = 15;//Since password length is 8 to 15 and +1 for null character.
  char *authentication_request_type = malloc(max_auth_req_type_len+1);
  if (authentication_request_type == NULL) {
    perror("Authentication Type malloc failed: ");
    exit(EXIT_FAILURE);
  }
  memset(authentication_request_type, 0, max_auth_req_type_len);
  strcpy(authentication_request_type, "register");
  
  //create a new window for accepting user inputs using ncurses library.
  //Initialize ncurse library.
  initscr();
  //Make cursor invisible by setting option to '0';if set to 1 then the cursor size is normal and if option is 2 then cursor is bigger in size.
  curs_set(0);
  
  WINDOW *menu = window_configuration(1);
  while( register_loop(menu, authentication_request_type, username, password, max_user_name_len, max_passwd_len, max_auth_req_type_len) == 1 )
  {
  }
  /*while( create_custom_window( menu, "chat_dashboard", guest_username, password, authentication_request_type, max_guest_username_length, max_passwd_len, max_auth_req_type_len ) == 1)
  {
      if( encode_or_decode_messages( socket_fd, guest_username, password, authentication_request_type, max_user_name_len, max_passwd_len, max_auth_req_type_len) == SUCCESS)
      {
        strcpy(authentication_request_type, "register");
        register_loop(socket_fd,menu, authentication_request_type, username, password, max_user_name_len, max_passwd_len, max_auth_req_type_len);
      }
  }*/
  
  //Start Conversation
  //start_chatting(
  delwin(menu);
  endwin();
  close(socket_fd);
  socket_fd = 0;
  return 0;
}
    
    
  
