/*
   Author: Ian Isely
   Date: 3/1/20
   Description: This scheduler follows the principle of Multi-Level Feedback Queue where
                new process are added to the high priority RR queue and executed over processes
                located in the low priority RR queue. A process is moved to the low prio queue
                once it has executed once already. This example makes use of two RR queues but
                more can be added seamlessly since each queue is located itself in a linked list.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>


//Ready List Declaration (List of Ready Lists)
typedef struct list_tag{
   pid_t my_id;
   struct list_tag * next;
   struct list_tag * prev;
} list_node;

typedef struct queue_tag{
   list_node * first;
   list_node * last;
   struct queue_tag * next;
} queue;


//Function Prototypes
void Queue_Init(queue*);
void List_Insert(queue*);
void Queue_Insert(queue*, pid_t);
void Queue_Remove(queue*);
void term_child();

//Global Variable for term_child signalhandler
int done = 0;


/* MAIN */
int main(int argc, char const *argv[])
{
   int qt = 50, i;
   pid_t child;
   queue RL;

   List_Insert(&RL);
   Queue_Init(&RL);
   Queue_Init(RL.next);

   if(argc > 2)
      qt = 1000*atoi(&argv[1][0]);
   else
   {
      printf("%s qt p1 p2 ... pN\n", argv[0]);
      exit(-1);
   } 

   /* Process Creation */

   for(i = 2; i < argc; i++)
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
   signal(SIGCHLD, term_child);
   while(RL.first != NULL || RL.next->first != NULL)
   {
      if(RL.first != NULL) //If high prio queue is not empty
      {
         kill(RL.first->my_id, SIGCONT);
         usleep(qt); //Time Quantum

         if(!done)
         {
            kill(RL.first->my_id, SIGUSR1);

            usleep(1000); //safe delay;

            child = RL.first->my_id;
            Queue_Remove(&RL);
            Queue_Insert(RL.next, child); //Move child to low prio queue
         }
         else
         {
            printf("Scheduler: A child has completed\n");
            Queue_Remove(&RL);
            done = 0;
         }
      }
      else //If high prio queue is empty
      {
         kill(RL.next->first->my_id, SIGCONT);
         usleep(qt); //Time Quantum

         if(!done)
         {
            kill(RL.next->first->my_id, SIGUSR1);

            usleep(1000); //safe delay;

            child = RL.next->first->my_id;
            Queue_Remove(RL.next);
            Queue_Insert(RL.next, child); //Reinsert to low prio queue
         }
         else
         {
            printf("Scheduler: A child has completed\n");
            Queue_Remove(RL.next);
            done = 0;
         }
      } 
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
   Function:    List_Insert
   Parameters:  queue*
   Description: Allocates memory for a new RR ready list
*/
void List_Insert(queue * rlp)
{
   rlp->next = (queue *) malloc(sizeof(queue));

   if(rlp->next == NULL)
   {
      printf("OUT OF MEMORY!!\n");
      exit(1);
   }
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
      printf("OUT OF MEMORY!!\n");
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


/*
   Function:    term_child
   Parameters:  NONE
   Description: Signalhandler for if scheduler recieves SIGCHLD from process while waiting
                in time quantum. Sets global bool value to true to indicate to remove
                process from queue.
*/
void term_child()
{
   signal(SIGCHLD, term_child);
   done = 1;
}
