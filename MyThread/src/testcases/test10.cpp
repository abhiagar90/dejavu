#include <stdio.h>
#include <unistd.h>
#include "MyThread.h"

void f();
void g();
void h();

int tid;

void f()
{
  int i=0,flag=0;
  while(1){
    printf("tid :%d in f: %d\n",getID(),i++);
    usleep(90000);
  }
}
void g()
{
  int i=0;
  int flag = 0;
  while(1){
    printf("tid :%d in g: %d\n",getID(),i++);
    usleep(90000);
  } 
}
void h()
{
  int i=0;
  int flag = 0;
  while(1){
    printf("tid :%d in h: %d\n",getID(),i++);
    if(i>3 && !flag){
	deleteThread(tid);
	flag = 1;		
    }
    usleep(90000);
  }
}



int main()
{
  int i=0;
  tid = create(f);
  create(g);
  create(h);
  start();
  printf("All children created\n");
  while(1)
  {
  }
}
