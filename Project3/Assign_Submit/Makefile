CC = gcc
CFLAGS=-pthread

#
all: phil_host phil

#
phil_host: 
	$(CC) $(CFLAGS) -o host host.c sem.c

#
phil:  
	$(CC) $(CFLAGS) -o phil phil.c sem.c

#
clean: 
	$(RM) host phil semaphore_* status_* barrier mutex counter *.o *~
