# Thread_pool

#program purpose

A Pthreads program that implements a “task queue.”
The main thread begins by starting a user-specified number of threads that immediately go to sleep
in a condition wait (the worker threads are idle at first).
The main thread generates tasks to be carried out by the other threads. A task is either an insert, a
delete, or a search a value in a sorted list (in ascending order) with no duplicates.

#implementation details

In the implementation of this project a thread pool is created to execute task produced by the main thread.

The main thread uses Task_queue function to create desired number of tasks.
Task_queue method calls enqueue method to add the randomly created task to the “task queue”.

The implementation of the “task queue” is simple. I used nodes and named the initial node as head. Every node has a reference of subsequent node. Thus, for adding a new task to queue, the program creates new node and makes it new head (initial node). For deletion, on the other hand, we go to the end of the queue and add the new node.

While main thread produces new tasks, worker threads dequeue a task an do it.
To ensure synchronization between the producer and consumers “pthread_cond_t task_cond ” variable is used.

Each time enqueue method adds a new task to the task queue it signals to the worker threads which are waiting in a loop on condition (if the head is null which means there is no new task). The loop for worker threads breaks if and only if the task queue is empty and the global variable flag is set to 0 (indicating there is no kore task left to be produced) by Task_queue metod.

After the signal worker threads call dequeue method, remove the first task in the queue and set their local variables (task_type , task_num, value). With a switch-case block the worker thread decides which operation to be performed on the sorted list and call the appropriate method.

(The sorted list implemented as a linked list. Thus; insert, delete, search and print_list methods are implemented accordingly.)

Also, there are two mutex variables (one for list operations, and one for task queue operations)

see programOutputs for examples
