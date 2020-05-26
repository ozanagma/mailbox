#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define COUNTRY_CNT 195
#define MESSAGE_SIZE 1024
#define READ_END 0
#define WRITE_END 1

typedef enum{
  ORDER_CONTINUES,
  ORDER_DONE
}E_COMMAND;

typedef struct{
  int country_code;
  E_COMMAND command;
  int order_count;
}S_MESSAGE_CONTENT;

typedef struct{
  long mes_type;
  S_MESSAGE_CONTENT content;
}S_MESSAGE_BUF;


int CountryOrderCount[COUNTRY_CNT];
pthread_t ThreadIDs[COUNTRY_CNT];
pthread_attr_t ThreadAtts[COUNTRY_CNT];
int PipeFileDes[2];

void* recive_order(void* param);

int main()
{
 printf("Server is started.\n");	


 S_MESSAGE_BUF receive_message;

 
 key_t key  = 2234;
 int msg_id = msgget(key, 0666 | IPC_CREAT);
 if(msg_id < 0){
   printf("msgget error.");
   printf("%s\n", strerror(errno));
   exit(1);
 }
 printf("IPC is created. Message id: %d \n", msg_id);
 if(pipe(PipeFileDes) == -1)
   printf("Pipe failed.");

 while(1){
   if(msgrcv(msg_id, (void*)&receive_message, MESSAGE_SIZE, 1, 0) > 0){
     if(CountryOrderCount[receive_message.content.country_code] == 0){
       printf("New country %d\n", receive_message.content.country_code);
       pthread_attr_init(&ThreadAtts[receive_message.content.country_code]);
       pthread_create(&ThreadIDs[receive_message.content.country_code], &ThreadAtts[receive_message.content.country_code], recive_order, NULL);


     }
     write(PipeFileDes[WRITE_END], (void*)&receive_message.content, sizeof(S_MESSAGE_CONTENT));   
   }
 }
  
  return 0;
}


void* recive_order(void* param)
{
  S_MESSAGE_CONTENT message_content;
  while(1){
    read(PipeFileDes[READ_END], (void*)&message_content, sizeof(S_MESSAGE_CONTENT));
    switch(message_content.command)
    {
    	case ORDER_CONTINUES:
    	  CountryOrderCount[message_content.country_code] += message_content.order_count;
    	  printf("Order received.");
    	  printf("Country code: %d", message_content.country_code);
    	  printf("Order count: %d", message_content.order_count);
    	  printf("Total order count: %d\n", CountryOrderCount[message_content.country_code]);
     	  break;
     	case ORDER_DONE:
     	  pthread_exit(NULL);
     	  break;
    }
  }
}

