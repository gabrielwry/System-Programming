# Multi Process 

This is the third home work of Emory CS 450 **System Programming** 2017 Fall, taught by Professor Ken Mandelberg.
Please note that this program is subject to Honor Code of **Emory University**.

## Note
This programming assignment is meant for practicing **multi process** programming and basic inter process communication using **pipe**. 
It takes a file from `stdin` and output the file to `stdout` with unique sorted words followed by their individual counts. 

## Code Strucutre
There are three main processes we need to execute in this program `parse`, `sort`, and `merge`. To manage these three processes, 
two different pipes are needed, namely, `parse-to-sort` stream, and `sort-to-merge` stream. <br>
**Parse Process** open the write-end of the `parse-to-sort` pipe, scan the whole text file word-by-word, filtering any word not alphabetical, shorter than 5, or longer than 35, 
then parse all words to lower case, all non-alphabetical characters are taken as word seperator. After all this is done, the write end of the
`parse-to-sort` end should be closed. <br>
**Sort Process** receives data from the `parse-sort` pipe and write data to the `sort-merge` pipe. It evokes the `/bin/sort` with `execv` 
system call, and write the sorting result to the `sort-merge` pipe.<br>
**Merge Process** opens the read end of `merge-sort` stream and put unique word from `sort-merge` pipe followed by their count to the `stdout`

## Summary
The first look at this assignment was very hard as this is the first time we dealt with multi process and inter process communication. <br>
Once you figured out the sequence of opening different ends of two pipes, the hardest part comes up (at least for me). the `/bin/sort` sort 
data from `stdin` and `stdout` so we need to use `dup2` to redirect the `fd` of `stdin` and `stdout` to the `fd` we needed, also `/bin/sort`
is sorting data seperated by lines so remeber to seperate all words in your `parse` process with a new line character.
