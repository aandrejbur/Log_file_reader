CC 			= 	gcc -pthread 
CFLAGS 		= 	-Wall
LDFLAGS 	=   # linking flags
SOURCES 	= 	main.c list_t.c usefull_utilities.c threads.c search_lib.c
OBJECTS 	= 	$(SOURCES:.c=.o)
EXECUTABLE 	= 	log_reader

.PHONY: all
all:  $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o: \
    $(CC) $(CFLAGS) $< -o $@

.PHONY:	clean
clean: 
	rm -rf *.o  log_reader Programmer_Commandments.txt Result.txt
