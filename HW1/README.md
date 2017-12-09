# Goldbach Test
This is the first home work of Emory CS 450 **System Programming** 2017 Fall, taught by Professor Ken Mandelberg.<br>
Please note that this program is subject to **Honor Code of Emory University**.

## Note
This code is a basic practice to C programming. It used **seive method** to test **Goldbach Conjecture** on even number greater than 5. 
**Goldbach Conjecture Theorem** states that: any integer larger than 2 can be written as the sum of three prime numbers. A more detailed
description can be found [here](http://mathworld.wolfram.com/GoldbachConjecture.html). <br>
**Seive Method** is a method to test prime number upto a given limit. A more detailed description can be found [here](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes)

## Code Structure
A structure `_seg` is defined as a linked list, that can hold an `int` array of size 256, and store information of the `prev` and
`next` segments that are linked to the current one. The `int` array was used as a `bitMap` for Seive Method. 
Helper functions to find the position of a bit on the `bitMap` and test for primality are defined as below: <br>
``` C
 seg* whichseg(int j);
 int whichint(int j);
 int whichbit(int j);
 void marknoprime(int j);
 int testprime(int j);
 ```
 The program will first take an argument for **Seive Test** up limit. Once it finished marking non-prime
 number on the `bitMap`, it will ask for user input of an even number larger than 5 for **Goldbach Test**.
 
 ## Summary
 The main purpose of this code is to practice `C` syntax and for better understanding the usage of `pointer` and bit operation. <br>
 The main challenge is to figure out how to define and use the linked-list like structure `_seg`, and 
 to use bit operation to test for primality and mark non-prime numbers.<br>
 Other than that, the remaining parts are simple implementation of two well-known algorithms.
