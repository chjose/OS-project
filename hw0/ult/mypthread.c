#include"mypthread.h"
#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>

typedef struct Node 
{
  mypthread_t* data;
  struct Node *next;
}node;

node *start = NULL,*current = NULL;

int thread_count = 0;

void insert(node *pointer, mypthread_t* data)
{
  node *temp = pointer;
  /* Iterate through the list till we encounter the last node.*/
  while(pointer->next!=temp)
  {
    pointer = pointer -> next;
  }
  /* Allocate memory for the new node and put data in it.*/
  pointer->next = (node *)malloc(sizeof(node));
  pointer = pointer->next;
  pointer->data = data;
  pointer->next = temp;
}

void init_list()
{
  node *temp;
  start = (node *)malloc(sizeof(node)); 
  temp = start;
  temp -> next = start;
  temp->data = (mypthread_t *)malloc(sizeof(mypthread_t)); 

  temp->data->status = MAIN_THREAD;
}

void mypthread_exit(void *retval)
{
  current->data->status = NOT_ACTIVE;
  mypthread_yield();
}

int promote_wait_thread(mypthread_t* thread)
{
  if(thread->status == WAIT)
  {
    if(thread->wait_for->status == NOT_ACTIVE)
    {
      thread->status = READY;
      return 1;
    }
  }
  return 0;
}

int mypthread_yield(void)
{
  ucontext_t* ab;
  node* temp = NULL;
  int count = 0;
 
  if (NULL == current)
  {
    ab = &start->data->a;
    current = start;
  }
  else
  {
    ab = &current->data->a;
  }

  temp = current->next;
  while ((temp->data->status != READY && 
          !promote_wait_thread(temp->data)) &&
         count++ != 100)
  {
    temp = temp->next;
  }

  if (temp->data->status == READY)
  {
    current = temp;
    swapcontext(ab,&((temp->data)->a));
  }
  else 
  {
    printf("\n!!!! NO THREADS IN READY STATE !!!!\n");
    current = NULL;
    setcontext(&start->data->a);
  }
}

int mypthread_join(mypthread_t thread, void **retval)
{
  node* temp = start->next;
  node* temp1 = current;

  if (thread.status != READY)
  {
    //printf("ERROR: mypthread_join() call requires a READY thread in the argument\n");
    return 1;
  }

  while(temp->data->thread_id != thread.thread_id)
  {
    temp=temp->next;
  }

  if (current == NULL)
  {
    current = start;
    temp1 = current;
  }

  if (temp->data->thread_id == thread.thread_id)
  {
    current->data->status = WAIT;
    current->data->wait_for = temp->data;
  }

  current = temp;
  swapcontext(&temp1->data->a,&temp->data->a);

  return 0;
}

int mypthread_create(mypthread_t *thread, 
                     const mypthread_attr_t *attr, 
                     void *(*start_routine) (void *), 
                     void *arg)
{
  ucontext_t * new_context;
 
  if (thread_count == 100)
  {
    printf("ERROR: Thread limit reached !!!");
    return 1;
  }

  if (NULL == thread)
  {
    printf("ERROR: Please pass a valid mypthread field.");
    return 1;
  }

  new_context = &(thread->a);
  if (NULL == start)
  {
    init_list();
  }

  getcontext(new_context);
  new_context->uc_link=0;
  new_context->uc_stack.ss_sp=malloc(MEM);
  new_context->uc_stack.ss_size=MEM;
  new_context->uc_stack.ss_flags=0;
  makecontext(new_context, (void (*)(void))start_routine, 1, arg);
  thread->status = READY;
  thread->thread_id = ++thread_count;
  insert(start,thread);

  return 0;
}
