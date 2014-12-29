#include <stdio.h>
#include <unistd.h>
#include "threadlib.h"

void f();
void g();
void h();
 
int tid ;

void f()
{
  int i=0;
  join(tid);
  while(1){
    printf("tid :%d in f: %d\n",getID(),i++);
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
void h()
{
  int i=0;
  while(i<20){
    printf("tid :%d in h: %d \n",getID(),i++);
    usleep(90000);
  }
}



int main()
{
  int i=0;
  create(f);
  create(g);
  tid = create(h);
  start();
  printf("All children created\n");
  while(1)
  {
  }
}
