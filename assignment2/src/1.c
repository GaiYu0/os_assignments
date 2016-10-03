#include<stdio.h>
#include<sys/wait.h>
#include<unistd.h>

int main(int argc, char *argv[]){
  int i, j, pid, exit_code;
  exit_code = 0;
  for(i = 0; i != 3; i++)
    if((pid = fork()) == 0){
      printf("i = %d\n", i);
      j = 0;
      while((j < i) && ((pid = fork()) == 0))
        j++;
      if(pid == 0)
        printf("j = %d\n", j);
      wait(&exit_code);
      _exit(j);
    }else
      wait(&exit_code);
  return 0;
}
