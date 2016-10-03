#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>

#define CHILDREN_NUMBER 10

int main(){
  int a, b, c, d, e;
  for(a = 0; a != CHILDREN_NUMBER; a++)
    if(fork() == 0){
      for(b = 0; b != 10; b++)
        for(c = 0; c != 10; c++)
          for(d = 0; d != 10; d++)
            for(e = 0; e != 10; e++)
              if(
                (a + 12 == b + c + d) &&
                (b + c + d == e + 8) &&
                (e + 8 == b + 6) &&
                (b + 6 == a + c + e) &&
                (a + c + e == d + 14) &&
                (d + 14 == c + 10)
              ){
                printf("%d %d %d\n", 4, a, 8);
                printf("%d %d %d\n", b, c, d);
                printf("%d %d %d\n", 2, e, 6);
              }
      exit(EXIT_SUCCESS);
    }
  int status;
  for(a = 0; a != CHILDREN_NUMBER; a++)
    wait(&status);
  return 0;
}
