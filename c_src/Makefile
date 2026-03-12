# Variables
CC = gcc
CFLAGS = -Wall -O2 -I.
LDFLAGS = -lprotobuf-c -lncurses -lsqlite3

# The final executable will be placed in the project's priv/ folder
TARGET = ../priv/chat_client

# Source files
SRCS = chat_application_clients.c chat_application.pb-c.c sql_database.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p ../priv
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
