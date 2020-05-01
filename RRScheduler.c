/*
   Author: Ian Isely
   Date: 2/26/20
   Description: This program acts as a process scheduler following the round robin (RR) philosophy.
                Every process is added to a RR ready list and allowed to execute for a given
                time quantum (qt) microseconds. If qt expires before the process is complete,
                the process is moved to the back of the ready list and the next is chosen.
                If the process completes within the time quantum (sends SIGCHLD to parent), then
                the scheduler will remove it completely from the ready list and proceed.
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
void term_child();

//Global Variable for term_child signalhandler
int done = 0;


/* MAIN */
int main(int argc, char const *argv[])
{
   int qt = 50, i;
   pid_t child;
   queue RL;

   Queue_Init(&RL);

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
   while(RL.first != NULL)
   {
      kill(RL.first->my_id, SIGCONT);
      usleep(qt); //Time Quantum

      if(!done)
      {
         kill(RL.first->my_id, SIGUSR1);

         usleep(1000); //safe delay;

         child = RL.first->my_id;
         Queue_Remove(&RL);
         Queue_Insert(&RL, child);
      }
      else
      {
         printf("Scheduler: A child has completed\n");
         Queue_Remove(&RL);
         done = 0;
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
