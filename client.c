#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <pthread.h>
#include <unistd.h>
#include <errno.h>


#define COUNTRY_CNT 195
#define MESSAGE_SIZE 1024

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

void* send_order(void* param);


int main(int argc, char* argv[])
{
  printf("Client is started.\n");	
  int msg_id;
  key_t key;
  S_MESSAGE_BUF send_message;

  
  send_message.mes_type 			= 1;
  send_message.content.country_code 		= 0;
  send_message.content.command 		= ORDER_CONTINUES;
  send_message.content.order_count    	= 10;


  key = 2234;
 
  msg_id = msgget(key, 0666);
 if(msg_id < 0){
   printf("msgget error.");
   printf("%s\n", strerror(errno));
   exit(1);
 }
  printf("IPC is get. Message id: %d \n", msg_id);
  
  while(1){
    sleep(1);
    char a = getchar();
    if(a == 'a'){
        if(fork() == 0){
          execl(".client", "selam", NULL);
          //execl("/usr/bin/gnome-terminal", "gnome-terminal", "-q", "-e", "./client", "turkey",(char*)0);
        }
        else
        {
          wait(NULL);
        }
    } 
  }
  
 return 0;
}
/*
void* send_order(void* param)
{
  
  for(int i = 0;i < 100;i++){
    if(i == 10)   send_message.content.command 		= ORDER_DONE;
    sleep(1);
    int err;
    err = msgsnd(msg_id, (void*)&send_message, MESSAGE_SIZE, 0);
    if(err < 0){
       printf("%s\n", strerror(errno));
    }

}*/
