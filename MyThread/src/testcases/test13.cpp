#include <stdio.h>
#include <unistd.h>
#include "MyThread.h"

void f();
void g();
void* h(void* a);

int no = 15; 
 
int tid ;

void f()
{
  int i=0;
	int flag = 0;
  JOIN(tid);
	int result; 
  while(1){
    
	if(!flag)
	{
	result = *( int * ) GetThreadResult(tid);
	flag =1;
	}
	printf("tid :%d in f: %d and result is %d\n",getID(),i++,result);
    usleep(90000);
  }
}
void g()
{
  int i=0;
  while(1){
    printf("tid :%d in g: %d\n",getID(),i++);
    usleep(90000);
  }
}

void* h(void* a)
{
  int i=0;
  while(i<20){
    printf("tid :%d in h: %d \n",getID(),i++);
    usleep(90000);
  }
 return &no;
}



int main()
{
    int i=0;
  create(f);
  create(g);
  tid = createWithArgs(h, NULL);
  start();
  printf("All children created\n");
  while(1)
  {
  }
}
