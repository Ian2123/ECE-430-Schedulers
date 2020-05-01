/*
   Author: Ian Isely
   Date: 3/1/20
   Description: This scheduler operates with the shortest job first policy. Processes are associated
                with a time frame equal to their executable name (the amount of times they print)
	        and placed in a sorted linked list accordingly. The processes are then allowed to
	        execute according to a FIFO policy.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>


//Ready List Declaration (sorted LL)
typedef struct list_tag{
   pid_t my_id;
   int time;
   struct list_tag * next;
} list_node;

typedef list_node * queue;


//Function Prototypes
void Queue_Init(queue*);
void Queue_Insert(queue*, int, pid_t);
void Queue_Remove(queue*);


/* MAIN */
int main(int argc, char const *argv[])
{
   int i, pstatus;
   pid_t child;
   queue RL;

   Queue_Init(&RL);

   if(argc < 2)
   {
      printf("%s p1 p2 ... pN\n", argv[0]);
      exit(-1);
   } 

   /* Process Creation */

   for(i = 1; i < argc; i++)
   {
      printf("Parent: Creating program %s\n", argv[i]);
      if((child = fork()) == 0)
         execl(argv[i], argv[i], NULL);
      else
         Queue_Insert(&RL, atoi(&argv[i][1]), child);
   }
   sleep(1); //safe delay

   /* Scheduling */

   printf("\nScheduler: Program scheduling beginning...\n");
   while(RL != NULL)
   {
      kill(RL->my_id, SIGCONT);
      wait(&pstatus);
      Queue_Remove(&RL);
      printf("Scheduler: A child has completed\n");
   }

   printf("Scheduler: Scheduling complete\n");
   return 0;
}


/* 
   Function:    Queue_Init
   Parameters:  queue*
   Description: Initializes a queue to NULL.
*/
void Queue_Init(queue * qp)
{
   *qp = NULL;
}


/*
   Function:    Queue_Insert
   Parameters:  queue*, pid_t
   Description: Inserts a new node at the end of a specified linked list 
                with pid_t data.
*/
void Queue_Insert(queue * rlp, int in_time, pid_t in_id)
{
   list_node * n = (list_node *) malloc(sizeof(list_node));

   if(n == NULL)
   {
      printf("OUT OF MEMORY!!");
      exit(1);
   }

   n->my_id = in_id;
   n->time = in_time;

   while(*rlp != NULL && (*rlp)->time < in_time)
      rlp = &((*rlp)->next);
   n->next = *rlp;
   *rlp = n;
}


/*
   Function:    Queue_Remove
   Parameters:  queue*
   Description: Removes the head node of the list and returns its pid_t
                data value.
*/
void Queue_Remove(queue * rlp)
{
   list_node *remove;

   if(rlp == NULL)
   {
      printf("Empty List\n");
      exit(1);
   }
   
   remove = *rlp;
   *rlp = (*rlp)->next;
   free(remove);
}
