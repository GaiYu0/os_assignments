#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>

#define TOTAL_CURRENCIES 5
#define CNY 0
#define EUR 1
#define GBP 2
#define JPY 3
#define USD 4

double table[TOTAL_CURRENCIES] = {1.0, 0.13336, 0.11186, 0.15009, 15.21564};
char *currencies[TOTAL_CURRENCIES] = {"CNY", "EUR", "GBP", "USD", "JPY"};

int main(int argc, char *argv[]){
  int currency_index;
  for(currency_index = 0; currency_index != TOTAL_CURRENCIES; currency_index++){
    if(strcmp(argv[1], currencies[currency_index]) == 0) break;
  }
  double value;
  value = atof(argv[2]);
  int i;
  printf("Conversion for: %s %s\n", argv[1], argv[2]);
  for(i = 0; i != TOTAL_CURRENCIES; i++)
    if(fork() == 0){
      double converted;
      converted = value / table[currency_index] * table[i];
      printf("%s %f\n", currencies[i], converted);
      exit(0);
    }
  int status;
  for(i = 0; i != TOTAL_CURRENCIES; i++)
    wait(&status);
  printf("End of conversion\n");
  return 0;
}
