#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

typedef unsigned char bool;

#define false 0
#define true 1
#define COUNTRY_CNT 195
#define MESSAGE_SIZE 1024
#define READ_END 0
#define WRITE_END 1

// ****** HASH FUNCTION IMPLEMENTATION  ******* //

struct S_COUNTRY{
   struct S_COUNTRY* next;
   char* name;
   int order_cnt;
   bool is_connection_open;
};

struct S_COUNTRY* CountryOrderCount[COUNTRY_CNT];

unsigned int hash(char* s)
{
   unsigned hashval;
   
   for(hashval = 0; *s != '\0'; s++)
     hashval = *s + 31 * hashval;
 
   return hashval % COUNTRY_CNT;
}

struct S_COUNTRY* lookup(char *s)
{
   struct S_COUNTRY* np;
   
   for(np = CountryOrderCount[ hash(s)]; np != NULL; np = np->next)
      if(strcmp(s, np->name) == 0)
         return np;
   
   return NULL;
}

struct S_COUNTRY* install(char* name)
{
   struct S_COUNTRY* np;
   struct S_COUNTRY* temp;
   unsigned int hashval;

   
   if((np = lookup(name)) == NULL){ // country not exist in hashtable
      np = (struct S_COUNTRY*) malloc(sizeof(*np));
      hashval = hash(name);
      if(CountryOrderCount[hashval] == NULL)  
         CountryOrderCount[hashval] = np;
      else{
      temp = CountryOrderCount[hashval];
      while(temp->next != NULL)
         temp = temp->next;
      temp->next = np;
      }
      np->next = NULL;
      np->name = name;
      np->order_cnt = 0;
      np->is_connection_open = false;
      return np;
   }
   else{
      return np;
   }   

}


// ****** END OF HASH FUNCTION IMPLEMENTATION  ******* //



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


pthread_t ThreadIDs[COUNTRY_CNT];
pthread_attr_t ThreadAtts[COUNTRY_CNT];
int PipeFileDes[COUNTRY_CNT][2];

void* recive_order(void* param);
void clear_msg_buffer(int msg_id);

int main()
{
 S_MESSAGE receive_message;
 
 key_t key  = 2234;
 int msg_id = msgget(key, 0666 | IPC_CREAT);  //new message queue is created.
 
 if(msg_id < 0){
   printf("msgget error.");
   printf("%s\n", strerror(errno));
   exit(1);
 }
 
  clear_msg_buffer(msg_id); // it erases data in the buffer just in case client sent message before server runs.

 for(int i = 0; i < COUNTRY_CNT; i++){ // All countries pipe's are created.
    if(pipe(PipeFileDes[i]) == -1)
      printf("%dth pipe failed.", i);    
 }

 while(1){

      if(msgrcv(msg_id, (void*)&receive_message, sizeof(S_MESSAGE)- sizeof(long), 1, 0) > 0) {
         int hash_value =  hash(receive_message.country_name);
         struct S_COUNTRY* country = install(receive_message.country_name); // put country in the hashtable.
         if(!country->is_connection_open){ // it handles 2 cases: 1) Current country's first order. 2) Current country close its connection and re-opens it.
            country->is_connection_open = true;
            pthread_attr_init(&ThreadAtts[hash_value]); // new thread.
            pthread_create(&ThreadIDs[hash_value], 
       	      &ThreadAtts[hash_value], 
       	      recive_order, (void*)&hash_value);
        }
        write(PipeFileDes[hash_value][WRITE_END], (void*)&receive_message, sizeof(S_MESSAGE));  //send data to related thread.
    }
 }
  
  return 0;
}


void* recive_order(void* param)
{
int hash_value = *(int*)param;
  S_MESSAGE message;

  while(1){
    read(PipeFileDes[hash_value][READ_END], (void*)&message, sizeof(S_MESSAGE));
    switch(message.command)
    {
    	case ORDER_CONTINUES:
    	   CountryOrderCount[hash_value]->order_cnt += message.order_count;
    	   printf("%-10s -- ", message.country_name);
    	   printf("Order Count: %-6d -- ", message.order_count);
    	   printf("Total Order Count: %d\n", lookup(message.country_name)->order_cnt);
     	   break;
     	case ORDER_DONE:
         lookup(message.country_name)->is_connection_open = false;
         printf("%-10s Communication end -- Total Order Count: %d\n", message.country_name, lookup(message.country_name)->order_cnt);
     	   pthread_exit(NULL);
     	  break;
    }
  }
}

void clear_msg_buffer(int msg_id)
{
	S_MESSAGE message;
	ssize_t ret;
	do{
	   ret = msgrcv(msg_id, (void*)&message , sizeof(S_MESSAGE) - sizeof(long), 1, IPC_NOWAIT);
	}while(errno != ENOMSG);	
	
	printf("Message buffer is cleaned.\n");
}

