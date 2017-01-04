List of Files:
=============
1. mapper.c
2. reducer.c
3. combiner.c
4. Makefile
5. Readme.txt
6. Input.txt

How to run:
==========

$ make clean
$ make
$ ./combiner Input.txt
      OR
$ ./combiner <Input File>

Design:
======

mapper.c    ->  reads words from "Input file" and write it out to "stdout" in the format (word,1)
reducer.c   ->  Words are stored in the data structures which are arranged in Linked List. Whenever a new word comes,
                it compares from the head of the linked list to the tail, if there is word match then count is 
                increased in the data structure of that word. If the first character of new word doesn't match
                with the first character of previous word, then the linked list is printed from head to tail.
                The memory is freed, and the new word with different character goes to head of the linked list.
combiner.c  ->  Has system calls, which connects mapper and reducer using PIPE. Creates different processes using
                fork() system calls.
