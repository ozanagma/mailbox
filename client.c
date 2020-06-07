#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define COUNTRY_CNT 195
#define MESSAGE_SIZE 1024
#define ORDER_REPEAT 100
#define MAX_ORDER_CNT 100000
#define MAX_SLEEP_TIME_MS 1000

typedef enum{
 MANUAL,
 AUTOMATIC
}E_MODE;

typedef enum{
  ORDER_CONTINUES,
  ORDER_DONE
}E_COMMAND;

typedef struct{
  long mes_type;
  char country_name[10];
  E_COMMAND command;
  int order_count;
}S_MESSAGE;

void* send_order(S_MESSAGE* param);

int msg_id;

int main(int argc, char* argv[])
{
  if(argc != 2){
	printf("Expected one(1) argument.\n");  
	exit(0);
  }
  
  char* p;
  E_MODE mode = strtol(argv[1], &p, 10);
  srand(time(0));
  
  if(mode != 0 && mode != 1){
  	printf("0 for Manual Mode or 1 for Automatic Mode.\n");
  	exit(0);
  }
  
  char* cntr_name = argv[0];
  int cntr_name_len = strlen(cntr_name);
  char cntr_name_cln[cntr_name_len - 2];
  memcpy(cntr_name_cln, cntr_name + 2, cntr_name_len -2);
  printf("Welcome %s\n", cntr_name_cln);

  key_t key = 2234;
  msg_id = msgget(key, 0666);
  if(msg_id < 0){
   printf("%s\n", strerror(errno));
   exit(0);
 }

  S_MESSAGE send_message;
  memcpy(send_message.country_name, cntr_name_cln, strlen(cntr_name_cln));
  send_message.mes_type = 1;

  int current_order = 0;
  int total_order = 0;
  
  switch(mode){
	case MANUAL:	
	   printf("You are in manual mode.\n");
	   printf("To order vaccine please enter order count.\n");
	   while(1){
	      scanf("%d", &current_order);
	      if(current_order != 0){
   	         total_order += current_order;
   	         send_message.command = ORDER_CONTINUES;
   	         send_message.order_count = current_order;
   	         send_order(&send_message);
   	         printf("Current order: %d ", current_order);
   	         printf("Total order: %d\n", total_order);
   	         printf("To continue order please enter new order count. " );
   	         printf("To stop order please enter \"0\".\n" );
	      }	      
   	      else{
   	         send_message.command = ORDER_DONE;
   	         send_message.order_count = current_order;
   	         send_order(&send_message);
   	         printf("Total order: %d Goodbye!\n", total_order);
   	         exit(0);
   	      }
   	   }
	break;
	case AUTOMATIC:
	   printf("You are in automatic mode.\n");
	   int sleep_time_ms;
	   for(int i = 0; i < ORDER_REPEAT; i++){
	      current_order = rand() % MAX_ORDER_CNT + 1;
  	      total_order += current_order;
   	      send_message.command = ORDER_CONTINUES;
   	      send_message.order_count = current_order;
   	      send_order(&send_message);
	      printf("Current order: %d ", current_order);
   	      printf("Total order: %d\n", total_order);
   	      sleep_time_ms = rand() % MAX_SLEEP_TIME_MS + 1; 
   	      usleep(sleep_time_ms);
	  }
	  printf("Total order: %d Goodbye!", total_order);		
	break; 
  }

 return 0;
}

void* send_order(S_MESSAGE* message)
{
   
   int err = msgsnd(msg_id, (void*)message, sizeof(S_MESSAGE)- sizeof(long), 0);
   if(err < 0)
      printf("%s\n", strerror(errno)); 
}
