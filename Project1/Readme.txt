Mapper program gets the input from a file and for each word in the file generates key value pairs in the form (word, 1). You can assume that the words are sorted according to the first letter but words that start with the same letter may not be sorted.

Reducer program gets the (word, 1) key-value pairs from the standard input as shown above and generates (word, total) key-value pairs on the standard output.

Combiner program gets its input from a file and for each word in the file generates the key value pair (word, count). Each key-value pair is output on a separate line. You should use fork, exec, and pipe system calls in the Combiner and reuse the Mapper and Reducer programs