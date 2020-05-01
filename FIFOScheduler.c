/*
   Author: Ian Isely
   Date: 3/1/20
   Description: This scheduler operates on the First In First Out policy. Processes are added to
                the ready list and allowed to execute from start to finish in the order they
                arrived.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>


//Ready List Declaration (double LL)
typedef struct list_tag{
   pid_t my_id;
   struct list_tag * next;
   struct list_tag * prev;
} list_node;

typedef struct{
   list_node * first;
   list_node * last;
} queue;


//Function Prototypes
void Queue_Init(queue*);
void Queue_Insert(queue*, pid_t);  //at end of list
void Queue_Remove(queue*);         //at head of list


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
         Queue_Insert(&RL, child);
   }
   sleep(1); //safe delay

   /* Scheduling */

   printf("\nScheduler: Program scheduling beginning...\n");
   while(RL.first != NULL)
   {
      kill(RL.first->my_id, SIGCONT);
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
   qp->first = qp->last = NULL;
}

/*
   Function:    Queue_Insert
   Parameters:  queue*, pid_t
   Description: Inserts a new node at the end of a specified linked list 
                with pid_t data.
*/
void Queue_Insert(queue * rlp, pid_t in_id)
{
   list_node * n = (list_node *) malloc(sizeof(list_node));

   if(n == NULL)
   {
      printf("OUT OF MEMORY!!");
      exit(1);
   }

   n->my_id = in_id;

   if(rlp->first == NULL)
   {
      n->next = n->prev = NULL;
      rlp->first = rlp->last = n;
   }
   else
   {
      n->prev = rlp->last;
      n->next = NULL;
      rlp->last->next = n;
      rlp->last = n;
   }
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
   
   remove = rlp->first;
   rlp->first = rlp->first->next;
   free(remove);
  
   if(rlp->first == NULL)
      rlp->last = NULL;
}
