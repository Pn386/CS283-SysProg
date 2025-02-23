CC = gcc
CFLAGS = -Wall -Wextra -g
SOURCES = dsh_cli.c dshlib.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = dsh

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: all
	chmod +x dsh
	bats ./bats/assignment_tests.sh ./bats/student_tests.sh

clean:
	rm -f $(OBJECTS) $(TARGET) *~