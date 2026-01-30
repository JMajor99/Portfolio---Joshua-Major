// MISRA C 
// 1. Making sure not to put functions within functions
// 2. Avoid stdio.h and local.h || Alternatives include stdlib.h and time.h
// 3. Use assert to find incorrect logic throughout the code
// 4. Name set numbers to make it easier to read
// 5. Avoid using exit

// Unavoidably mmap and smh_open were used as we need to directly access the shared memory to find discrepencies
// Some exit(1) were required as occasionally the cores would need to be dumped when using assert(0) instead

#include <assert.h> 
#include <stdlib.h>
#include <fcntl.h>    
#include <sys/mman.h>  
#include <unistd.h>   
#include <pthread.h>  
#include <stdint.h>  
#include <string.h>  
#include <sys/stat.h>  
#include <errno.h>     
#include <ctype.h>

#define MSG_LEN 128
#define FLOOR_BYTE_LENGTH 4
#define STATUS_BYTE_LENGTH 8
#define READ_WRITE 0666

const char *valid_statuses[] = {"Opening", "Open", "Closing", "Closed", "Between"};

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char current_floor[FLOOR_BYTE_LENGTH];
    char destination_floor[FLOOR_BYTE_LENGTH];
    char status[STATUS_BYTE_LENGTH];
    uint8_t open_button;
    uint8_t close_button;
    uint8_t door_obstruction;
    uint8_t overload;
    uint8_t emergency_stop;
    uint8_t individual_service_mode;
    uint8_t emergency_mode;
} car_shared_mem;

int shm_fd;
car_shared_mem *shared_mem_ptr;
char shm_name[MSG_LEN];

void print_f(const char *print) {
    write(STDOUT_FILENO, print, strlen(print)); 
    write(STDOUT_FILENO, "\n", 1);
}

int check_valid_status(const char *status)
{
    int flag = 0;
    for (int i = 0; i < 5; i++)
    {
        if (strcmp(status, valid_statuses[i]) == 0)
        {
            flag ++;
        }
    }
    assert(flag == 1 || flag == 0);
    if (flag != 1)
    {
        flag = -1;
    }
    return flag;
}

int valid_floors(const char* floors)   // Used for finding valid basement peramiter
{
    int check = 0;
    if (strlen(floors) > 3)
    {
        check = -1;
    }
    else if (isalpha(floors[0]) && isdigit(floors[1]) && isdigit(floors[2]))
    {
        if (floors[0] != 'B' || isalpha(floors[1]) || isalpha(floors[2]))   // Checking if the first value is B and the others are digits
        {
            check = -1;
        }
    }
        else if (isalpha(floors[1]) || isalpha(floors[2]))  // Making sure that the last 2 values are number not letters eg. B 12 not B BA etc.
    {
        check = -1;
    }
    else if (floors == 0)
    {
        check = -1;
    }
    assert(check == -1 || check == 0);
    return check;
}

int valid_value(car_shared_mem *shared_mem_ptr)
{
    int check = 0;
    if (shared_mem_ptr->open_button != 1 && shared_mem_ptr->open_button != 0)
    {
        check = -1;
    }
    else if (shared_mem_ptr->close_button != 1 && shared_mem_ptr->close_button != 0)
    {
        check = -1;
    }
    else if (shared_mem_ptr->door_obstruction != 1 && shared_mem_ptr->door_obstruction != 0)
    {
        check = -1;
    }
    else if (shared_mem_ptr->overload != 1 && shared_mem_ptr->overload != 0)
    {
        check = -1;
    }
    else if (shared_mem_ptr->emergency_stop != 1 && shared_mem_ptr->emergency_stop != 0)
    {
        check = -1;
    }
    else if (shared_mem_ptr->individual_service_mode != 1 && shared_mem_ptr->individual_service_mode != 0)
    {
        check = -1;
    }
    else if (shared_mem_ptr->emergency_mode != 1 && shared_mem_ptr->emergency_mode != 0)
    {
        check = -1;
    }
    assert(check == 0 || check == -1);
    return check;
}

int main(int argc, char *argv[])
{
    if (argc != 2) 
    {
        print_f("Usage: ./safety {car name}\n");
        exit(1);
    }
    
    int valid_floor_flag = 0;

    strcpy(shm_name, "/car"); // Copies /car into the shm_name
    strcat(shm_name, argv[1]); // Finalises the name of the car

    shm_fd = shm_open(shm_name, O_RDWR, READ_WRITE);    // Making a POSIX shared memory allocation for access between processes
    if (shm_fd == -1) 
    {
        if (errno == ENOENT)    // ENOENT is the error given by no shared memory
        {
            print_f("Unable to access car.");   // TODO: Need to add a way to include the name of the car as well to this
            exit(1);
        }
        else
        {
            print_f("shm_open failed");
            exit(1);
            
        }
    }

    if (ftruncate(shm_fd, sizeof(car_shared_mem)) == -1)  // Resizing the memory from 0 bytes to the size needed for the entire typedef struc
    { 
        print_f("ftruncate failed\n");
        exit(1);
    }

    shared_mem_ptr = mmap(NULL, sizeof(car_shared_mem), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // Allows other code to access and modify this info
    if (shared_mem_ptr == MAP_FAILED) 
    {
        print_f("mmap failed\n");
        exit(1);
    }
 
    while(1)
    {            
        if (shared_mem_ptr->door_obstruction == 1 && strcmp(shared_mem_ptr->status, "Closing") == 0)
        {
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            shared_mem_ptr->close_button = 0;
            strncpy(shared_mem_ptr->status, "Opening", sizeof(shared_mem_ptr->status));
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
        }
        if (shared_mem_ptr->emergency_stop == 1 && shared_mem_ptr->emergency_mode != 1)
        {
            print_f("The emergency stop button has been pressed!"); 
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            shared_mem_ptr->emergency_mode = 1;
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
        }
        if (shared_mem_ptr->overload == 1  && shared_mem_ptr->emergency_mode != 1)
        {
            print_f("The overload sensor has been tripped!"); 
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            shared_mem_ptr->emergency_mode = 1;
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);            
        }
        if (valid_floors(shared_mem_ptr->current_floor) == -1 || valid_floors(shared_mem_ptr->destination_floor) == -1)
        {
            valid_floor_flag = -1;
        }
        if (shared_mem_ptr->emergency_mode != 1 && (valid_floor_flag == -1  || check_valid_status(shared_mem_ptr->status) == -1 || valid_value(shared_mem_ptr) == -1 || (shared_mem_ptr->door_obstruction == 1 && (strcmp(shared_mem_ptr->status, "Opening") != 0 && strcmp(shared_mem_ptr->status, "Closing") != 0))))
        {
            print_f("Data consistency error!");
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            shared_mem_ptr->emergency_mode = 1;
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);            
        }
    }
    return 0;
}