#include <stdio.h>
#include <unistd.h>
#include "MyThread.h"

void f();
void g();
void h();

void f()
{
  int i=0;
    printf("tid :%d in f: %d\n",getID(),i++);
    usleep(90000);
}

void g()
{
  int i=0;
  printf("tid :%d in g: %d\n",getID(),i++);
  usleep(90000);
}

void h()
{
  int i=0;
  printf("tid :%d in h: %d\n",getID(),i++);
while(1) {
	
  }
}


//1 - Threads getting created
//2 - Terminating version

int main()
{
  int i=0;
  create(f);
  create(g);
  create(h);
  start();
  printf("All children created\n");
}
