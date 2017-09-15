/* C program implementing goldbach with seive method */
/* This program is my own code. Runye Wang*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

 typedef struct _seg {
 	int bits[256];//initialize all array elements to 0
 	struct _seg *next, *prev;
 	}seg;

 #define BITSPERSEG  (8*256*sizeof(int)) //8*256*4 = 8192 bits per seg

 seg* whichseg(int j);
 int whichint(int j);
 int whichbit(int j);
 void marknoprime(int j);
 int testprime(int j);
 void seive(int j);
 void testGoldbach(int j);
 int countPrimes(int j);
 seg *head,*tail,*pt;

 void main(int argc, char *argv[]){

 	
    int      i;
    int howmany;
    int num_odd;
    int num_prime;
    int howmany_seg;
    int test;

    if (argc == 2) 
    	sscanf(argv[1],"%d",&howmany);
    else 
        scanf("%d",&howmany);
    //=======Code following sample=========//
    printf("Calculating odd primes up to %d...\n",howmany);
    num_odd = howmany /2; // only need odd number
    howmany_seg = num_odd / BITSPERSEG;  // Number of segments 
    head= (seg*) malloc(sizeof(seg));
    pt=head;
    for (i=0;i<howmany_seg;i++) 
    { 
        pt->next = (seg*) malloc(sizeof (seg));
        pt->next->prev = pt; 
        pt = pt->next; //set up next and prev pointer
        //pt->bits[0]=1;
    }

    pt->next = NULL;
    tail = pt;
    /*
    head->bits[0]=2;
    seg *tmp;
    tmp = whichseg(9000);
    printf("Debugging %d\n",head->bits[0]);
    */
    
    //printf("The next node of head has %d on it's first bit. \n",tmp->bits[0] );
    //printf("seiving\n");
    
    seive(howmany);
    num_prime = countPrimes(howmany);
    printf("Found %d odd primes\n", num_prime);
    printf("Enter Even Numbers >5 for Goldbach Tests: \n");
    scanf("%d",&test);
    while(test%2 != 0 || test <= 5){ //keep asking for input if input is not even number >5
    	scanf("%d",&test);
    }
    testGoldbach(test);


 }

 seg* whichseg(int j)
 {
 	seg *p;
 	int i;
 	int numAlloc;
 	p=head;
 	numAlloc = j/BITSPERSEG; //how many times to move the pointer 

 	for(i=0;i<numAlloc;i++)
 	{	
 		p=p->next;
 		//printf("moving %d\n",i);
 	}

 	return p;	
 }

 int whichint(int j)
 {
 	return (j%BITSPERSEG)/(8*sizeof(int));
 }
 int whichbit(int j)
 {
 	return (j%BITSPERSEG)%(8*sizeof(int));
 }
 void marknoprime(int j)
 {
 	seg *p;
 	int a;
 	int b;
 	//printf("Debugging to see what goes into marknoprime, %d\n",j);
 	p = whichseg(j);
 	a = whichint(j);
 	//printf("Debugging, a is %d\n",a);
 	//printf("Size of int %d\n",sizeof(int) );
 	b = whichbit(j);
 	p->bits[a] |= 1<<b;
 	//p->bits[0]=1;
 	//printf("Succeed uptil now\n");
 	 
  }
 int testprime (int j)
 {
 	seg *p;
 	int a;
 	int b;
 	//printf("Debugging to see what goes into testprime, %d\n",j);
 	p = whichseg(j);
 	a = whichint(j);
 	b = whichbit(j);

 	return (p->bits[a] >> b)&1;
 }
 void seive(int j)
 {
 	
 	marknoprime(0);//odd number 1 is not prime ==> bit position 0 is not prime
 	//printf("testprime result for 2 is %d",testprime(2));
 	int mul;
 	int i;	
 	for(i=3;(i*i)<j;i+=2)
 	{
 		if(testprime((i-1)/2)==0)
 		{
 			
 			for(mul=i; (mul*i)<=j; mul+=2)
 			{
 				marknoprime((mul*i-1)/2);
 			}
 		}
 	}

 }

 int countPrimes(int j)
 {
 	int i;
 	int count=0; // only count odd primes
 	for (i = 3; i<j;i+=2){
 		if(testprime((i-1)/2) == 0){
 			count++;
 			//printf("%d\n",i );
 		}
 	}
 	return count;
 }

 void testGoldbach(int j)
 {
 	int iteration,int_i,int_k,bit_i,bit_k,count,max;
 	seg *seg_i,*seg_k;
 	count = 0;
 	max = 0;
 	seg_i=whichseg(3/2);
 	seg_k=whichseg((j-3)/2);
 	int_i = whichint(3/2);
 	int_k = whichint((j-3)/2);
 	bit_i = whichbit(3/2);
 	bit_k = whichbit((j-3)/2);

 	for(iteration=1;iteration<=j/4;iteration++){
 		/*
 			Start from iteration = 1, at bit map is int 3,
 			only test till the half of j, in terms of number of odd, is j/4,
 		*/
 		printf("wtf?? %d\n",iteration*2+1);
 		if(!(seg_i->bits[int_i] & 1<<bit_i) & !(seg_k->bits[int_k] & 1<<bit_k) ){//if both are prime
 			
 			count++;
 			if ((2*iteration+1)>max){
 				max = 2*iteration+1;
 			}
 		}
 		bit_i++;
 		bit_k--;
 		
 		if(bit_i == 8*sizeof(int)){//if reach to last bit of a int, jump to next int
 			bit_i = 0;
 			int_i++;
 			//printf("Reset bit_i at %d iteratioon\n",iteration );
 			if(int_i == 256){//if reach to last int, jump to next seg, a better way is to use sizeof tho;
 				int_i = 0;
 				//printf("Reset bit_k at %d iteratioon\n",iteration );
 				seg_i=seg_i->next;
 			}
 		}
 		if(bit_k == -1){//if reach to last bit of a int, jump to next int
 			bit_k = 8*sizeof(int)-1;
 			int_k--;
 			//printf("Reset int_i at %d iteratioon\n",iteration );
 			if(int_k == -1){//if reach to head of seg, jump to prev seg, a better way is to use sizeof tho;
 				int_k = 256-1;
 				//printf("Reset int_k at %d iteratioon\n",iteration );
 				seg_k=seg_k->prev;
 			}
 		}
 		
 		
 	}
 	printf("%d = %d + %d (out of %d solutions)\n",j,max,j-max,count);
 	
 }