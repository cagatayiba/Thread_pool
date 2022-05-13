#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>



int flag = 1;

typedef struct list_node_s{
    int data;
    struct list_node_s * next;
}node_list;

typedef struct task_node_s{
    int task_num;
    int task_type;
    int value;
    struct  task_node_s* next;
}node_task;


node_list* search(int);
int insert(int);
int delete(int);
int print_list();
void Task_queue(int n); //generate random tasks for the task queue
void Task_enqueue(int task_num, int task_type, int value); //insert a new task into task queue
int Task_dequeue(int* task_num_p, int* task_type_p, int* value_p); //take a task
void* execute_task(void* args);


node_list *root = NULL; 
node_task* head = NULL;

pthread_mutex_t task_mutex;
pthread_cond_t task_cond;
pthread_mutex_t  list_mutex;


int main(int argc, char* argv[]){



    int thread_num;
    int task_num ;
    printf("please enter the desired thread number : \n");
    scanf("%d", &thread_num);
    printf("please enter the desired task number : \n");
    scanf("%d", &task_num);


    clock_t start, end;
    double cpu_time_used;

    start = clock();

    pthread_mutex_init(&list_mutex, NULL);
    pthread_mutex_init(&task_mutex, NULL);
    pthread_cond_init(&task_cond, NULL);

    pthread_t th[thread_num];
    int i;
    for (i = 0; i < thread_num; i++) {
        if (pthread_create(&th[i], NULL, &execute_task, NULL) != 0) {
            perror("Failed to create the thread");
        }
    }

   
    Task_queue(task_num);
    

    for (i = 0; i < thread_num; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join the thread");
        }
    }
    pthread_mutex_destroy(&list_mutex);
    pthread_mutex_destroy(&task_mutex);
    pthread_cond_destroy(&task_cond);
    print_list();

    end = clock();

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("time elapsed is: %lf\n", cpu_time_used);
    
    return 0;
    

}

node_list* search(int key){
    
    node_list* iter = root;
    while(iter != NULL){
        if(iter->data == key){
            
            return iter;
        }
        iter = iter->next;
    } 
    
    return NULL;
}

int insert(int given){
    if(search(given) != NULL){
        return 0;
    }

    // initializing the node to be added
    node_list *new = (node_list*)malloc(sizeof(node_list));
    new->data = given;
    new->next = NULL;
    
    //if the node to be added is the first node
    if(root == NULL){
       root = new;
    }else{

        //new node is smaller than the root 
        if(root->data > given){
                new->next = root;
                root = new;
        }
        
        else{
            node_list* iter = root;
            // finding yhe position 
            while(iter->next != NULL && iter->next->data < given){
                iter = iter->next;
            }
            //add the new node
            new->next = iter->next;
            iter->next = new;

        }
    }
    return 1;
}



int delete(int key){
    //list is empty
    if(root == NULL){
        return 0;
    }
    //root is deleted
    if(root->data == key){
        // list's length is zero
        if(root->next == NULL){
            root =  NULL;
            
        }
        else
        {
            node_list* temp = root;
            root = root->next;
            temp->next = NULL;
            free(temp);
        }
        return 1;

    }else{
        // an element other than root is deleted
        node_list* iter = root->next;
        node_list* before = root;
        while(iter != NULL ){
            if(iter->data == key){
                node_list* temp = iter;
                before->next = iter->next;
                iter->next = NULL;
                free(temp);
                return 1;
            }
            iter = iter->next;
            before = before->next;
        }
        
    }
    return 0;

}


int print_list(){
    printf("the list is : \n");
    node_list* iter = root;
    while (iter != NULL)
    {
        printf("%d ", iter->data);
        iter = iter->next;
    }
    printf("\n");
      
}

void Task_enqueue(int task_num, int task_type, int value){
    // create new task
    node_task* new_task = (node_task*)malloc(sizeof(node_task));
    new_task->task_num = task_num;
    new_task->task_type = task_type;
    new_task->value = value;
    new_task->next = NULL;
    
    pthread_mutex_lock(&task_mutex);
    // if there is no element in queue
    if(head == NULL){
       
        head= new_task;
    }
    else
    {
        
        node_task* iter = head;
        //finding the last element in the queue
        while(iter->next != NULL){
            iter = iter->next;
        }
        // link the  new_task to the last position
        iter->next = new_task;


    }
    pthread_mutex_unlock(&task_mutex);
    pthread_cond_signal(&task_cond);
    
}

void Task_queue(int n){
    srand(time(NULL));
    for(int i=0 ; i<n ; i++){
        int task_num = i;
        int task_type = rand() % 3;
        int value = rand() % 5;
        
        
        Task_enqueue(task_num, task_type, value);

        

    }
    flag = 0;
    pthread_cond_broadcast(&task_cond);
}


int Task_dequeue(int* task_num_p, int* task_type_p, int* value_p){
    
    if(head == NULL){
        return 0;
    }else{
        *task_num_p = head->task_num;
        *task_type_p = head->task_type;
        *value_p = head->value;
        node_task* temp = head;
        head = head->next;
        temp->next = NULL;
        free(temp);
        return 1;
    }
   


}


void* execute_task(void* args){
    int task_num, task_type, value;
    while(1){

        //if task queue is empty wait on condition else call Task_dequeue method
        pthread_mutex_lock(&task_mutex);
        while (head == NULL) {
                // no more task and queue is empty
                if(flag == 0){
                    pthread_mutex_unlock(&task_mutex);
                    return NULL;
                }
                pthread_cond_wait(&task_cond, &task_mutex);
        }
    
        Task_dequeue(&task_num, &task_type, &value);
        pthread_mutex_unlock(&task_mutex);
    
        switch(task_type)
        {
            case 0: 
                pthread_mutex_lock(&list_mutex);
                if(insert(value) != 0){
                    pthread_mutex_unlock(&list_mutex);
                    printf("task %d: %d is inserted\n", task_num, value);
                }else{
                    pthread_mutex_unlock(&list_mutex);
                    printf("task %d: %d cannot be inserted\n", task_num, value);
                }
                break;
            case 1:
                pthread_mutex_lock(&list_mutex);
                if (delete(value) != 0){
                    pthread_mutex_unlock(&list_mutex);
                    printf("task %d: %d is deleted\n" , task_num, value);
                }else{
                    pthread_mutex_unlock(&list_mutex);
                    printf("task %d: %d cannot be deleted\n", task_num, value);
                }
                break;
            case 2:
                pthread_mutex_lock(&list_mutex);
                if(search(value) != NULL){
                    pthread_mutex_unlock(&list_mutex);
                    printf("task %d: %d is found\n", task_num, value);
                }else{
                    pthread_mutex_unlock(&list_mutex);
                    printf("task %d: %d is not found\n", task_num, value);
                }
                break;

        }

    }
    
}