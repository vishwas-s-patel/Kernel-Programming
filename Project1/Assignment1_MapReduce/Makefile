CC = gcc

#
all: mapReduce

#
mapReduce:  combiner mapper reducer
	$(CC) $(CFLAGS) -o combiner combiner.o

#
combiner.o:  combiner.c 
	$(CC) -c combiner.c

#
mapper:  mapper.c
	$(CC) -o mapper mapper.c

#
reducer:  reducer.c
	$(CC) -o reducer reducer.c

#
clean: 
	$(RM) combiner mapper reducer *.o *~
