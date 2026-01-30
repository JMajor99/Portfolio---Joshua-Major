#include <stdio.h>
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

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char current_floor[4];
    char destination_floor[4];
    char status[8];
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
char shm_name[256];

void press_open_button(car_shared_mem *shared_mem_ptr) {
    pthread_mutex_lock(&shared_mem_ptr->mutex);
    shared_mem_ptr->open_button = 1;
    pthread_cond_signal(&shared_mem_ptr->cond);  
    pthread_mutex_unlock(&shared_mem_ptr->mutex);
}

void press_close_button(car_shared_mem *shared_mem_ptr) {
    pthread_mutex_lock(&shared_mem_ptr->mutex);
    shared_mem_ptr->close_button = 1;
    pthread_cond_signal(&shared_mem_ptr->cond);  
    pthread_mutex_unlock(&shared_mem_ptr->mutex);
}

void press_stop_button(car_shared_mem *shared_mem_ptr) {
    pthread_mutex_lock(&shared_mem_ptr->mutex);
    shared_mem_ptr->emergency_stop = 1;
    pthread_cond_signal(&shared_mem_ptr->cond); 
    pthread_mutex_unlock(&shared_mem_ptr->mutex);
}

void service_on(car_shared_mem *shared_mem_ptr) {
    pthread_mutex_lock(&shared_mem_ptr->mutex);
    shared_mem_ptr->individual_service_mode = 1;
    shared_mem_ptr->emergency_mode = 0; 
    pthread_cond_signal(&shared_mem_ptr->cond); 
    pthread_mutex_unlock(&shared_mem_ptr->mutex);
}

void service_off(car_shared_mem *shared_mem_ptr) {
    pthread_mutex_lock(&shared_mem_ptr->mutex);
    shared_mem_ptr->individual_service_mode = 0;
    pthread_cond_signal(&shared_mem_ptr->cond);  
    pthread_mutex_unlock(&shared_mem_ptr->mutex);
}

void move_up(car_shared_mem *shared_mem_ptr) //  TODO: Both move up and down could be potentially be optimised with casting it to atoi then changing the val and recasting
{
    pthread_mutex_lock(&shared_mem_ptr->mutex);
    if (isalpha(shared_mem_ptr->destination_floor[0]) && isdigit(shared_mem_ptr->destination_floor[1]) && isdigit(shared_mem_ptr->destination_floor[2])) 
    {
        if (shared_mem_ptr->destination_floor[2] == '0') 
        {
            shared_mem_ptr->destination_floor[2] = '9';
            int number = atoi(&shared_mem_ptr->destination_floor[1]);
            number--;
            snprintf(&shared_mem_ptr->destination_floor[1], 12, "%d", number); 
        } 
        else 
        {
            int number = atoi(&shared_mem_ptr->destination_floor[2]);
            number--;
            snprintf(&shared_mem_ptr->destination_floor[2], 12, "%d", number);  
        }
    }
    else if (isalpha(shared_mem_ptr->destination_floor[0]) && isdigit(shared_mem_ptr->destination_floor[1])) 
    {
        if (shared_mem_ptr->destination_floor[1] == '1') 
        {
            shared_mem_ptr->destination_floor[0] = '0';  
            shared_mem_ptr->destination_floor[1] = '0';
            shared_mem_ptr->destination_floor[2] = '1';
        } 
        else 
        {
            int number = atoi(&shared_mem_ptr->destination_floor[1]);
            number--;
            snprintf(&shared_mem_ptr->destination_floor[1], 12, "%d", number);
        }
    }
    else if (isdigit(shared_mem_ptr->destination_floor[0]) && isdigit(shared_mem_ptr->destination_floor[1]) && isdigit(shared_mem_ptr->destination_floor[2])) 
    {
        if (shared_mem_ptr->destination_floor[2] == '9') 
        {
            if (shared_mem_ptr->destination_floor[1] == '9') {
                shared_mem_ptr->destination_floor[1] = '0';
                shared_mem_ptr->destination_floor[2] = '0';
                int number = atoi(&shared_mem_ptr->destination_floor[0]);
                number++;
                snprintf(&shared_mem_ptr->destination_floor[0], 12, "%d", number);  
            } 
            else 
            {
                shared_mem_ptr->destination_floor[2] = '0';
                int number = atoi(&shared_mem_ptr->destination_floor[1]);
                number++;
                snprintf(&shared_mem_ptr->destination_floor[1], 12, "%d", number);  
            }
        } 
        else 
        {
            int number = atoi(&shared_mem_ptr->destination_floor[2]);
            number++;
            snprintf(&shared_mem_ptr->destination_floor[2], 12, "%d", number); 
        }
    }
    else if (isdigit(shared_mem_ptr->destination_floor[0]) && isdigit(shared_mem_ptr->destination_floor[1])) 
    {
        if (shared_mem_ptr->destination_floor[1] == '9') 
        {
            if (shared_mem_ptr->destination_floor[0] == '9') 
            {
                shared_mem_ptr->destination_floor[0] = '1';
                shared_mem_ptr->destination_floor[1] = '0';
            } 
            else 
            {
                shared_mem_ptr->destination_floor[1] = '0';
                int number = atoi(&shared_mem_ptr->destination_floor[0]);
                number++;
                snprintf(&shared_mem_ptr->destination_floor[0], 12, "%d", number); 
            }
        } 
        else 
        {
            int number = atoi(&shared_mem_ptr->destination_floor[1]);
            number++;
            snprintf(&shared_mem_ptr->destination_floor[1], 12, "%d", number); 
        }
    }
    else if (isdigit(shared_mem_ptr->destination_floor[0])) 
    {
        if (shared_mem_ptr->destination_floor[0] == '9') {
            shared_mem_ptr->destination_floor[0] = '1';
            shared_mem_ptr->destination_floor[1] = '0';
        } 
        else 
        {
            int number = atoi(&shared_mem_ptr->destination_floor[0]);
            number++;
            snprintf(&shared_mem_ptr->destination_floor[0], 12, "%d", number);  
        }
    }
    pthread_cond_signal(&shared_mem_ptr->cond);  
    pthread_mutex_unlock(&shared_mem_ptr->mutex);
    }

void move_down(car_shared_mem *shared_mem_ptr) {
    pthread_mutex_lock(&shared_mem_ptr->mutex);
    if (isalpha(shared_mem_ptr->destination_floor[0]) && isdigit(shared_mem_ptr->destination_floor[1]) && isdigit(shared_mem_ptr->destination_floor[2])) 
    {
        if (shared_mem_ptr->destination_floor[2] == '9') 
        {
            shared_mem_ptr->destination_floor[2] = '0';
            int number = atoi(&shared_mem_ptr->destination_floor[1]);
            number++;
            snprintf(&shared_mem_ptr->destination_floor[1], 12, "%d", number); 
        } 
        else 
        {
            int number = atoi(&shared_mem_ptr->destination_floor[2]);
            number++;
            snprintf(&shared_mem_ptr->destination_floor[2], 12, "%d", number);  
        }
    }
    else if (isalpha(shared_mem_ptr->destination_floor[0]) && isdigit(shared_mem_ptr->destination_floor[1])) 
    {
        if (shared_mem_ptr->destination_floor[1] == '9') 
        {
            shared_mem_ptr->destination_floor[1] = '0'; 
        } 
        else 
        {
            int number = atoi(&shared_mem_ptr->destination_floor[1]);
            number++;
            snprintf(&shared_mem_ptr->destination_floor[1], 12, "%d", number);  
        }
    }
    else if (isdigit(shared_mem_ptr->destination_floor[0]) && isdigit(shared_mem_ptr->destination_floor[1]) && isdigit(shared_mem_ptr->destination_floor[2])) 
    {
        if (shared_mem_ptr->destination_floor[2] == '0') 
        {
            if (shared_mem_ptr->destination_floor[1] == '0') 
            {
                shared_mem_ptr->destination_floor[1] = '9';
                shared_mem_ptr->destination_floor[2] = '9';
                int number = atoi(&shared_mem_ptr->destination_floor[0]);
                number--;
                snprintf(&shared_mem_ptr->destination_floor[0], 12, "%d", number); 
            } 
            else 
            {
                shared_mem_ptr->destination_floor[2] = '9';
                int number = atoi(&shared_mem_ptr->destination_floor[1]);
                number--;
                snprintf(&shared_mem_ptr->destination_floor[1], 12, "%d", number);  
            }
        } 
        else 
        {
            int number = atoi(&shared_mem_ptr->destination_floor[2]);
            number--;
            snprintf(&shared_mem_ptr->destination_floor[2], 12, "%d", number);  
        }
    }
    else if (isdigit(shared_mem_ptr->destination_floor[0]) && isdigit(shared_mem_ptr->destination_floor[1])) 
    {
        if (shared_mem_ptr->destination_floor[1] == '0') 
        {
            if (shared_mem_ptr->destination_floor[0] == '0') 
            {
                shared_mem_ptr->destination_floor[0] = '9';
                shared_mem_ptr->destination_floor[1] = '9';
            } 
            else 
            {
                shared_mem_ptr->destination_floor[1] = '9';
                int number = atoi(&shared_mem_ptr->destination_floor[0]);
                number--;
                snprintf(&shared_mem_ptr->destination_floor[0], 12, "%d", number); 
            }
        } 
        else 
        {
            int number = atoi(&shared_mem_ptr->destination_floor[1]);
            number--;
            snprintf(&shared_mem_ptr->destination_floor[1], 12, "%d", number); 
        }
    }
    else if (isdigit(shared_mem_ptr->destination_floor[0])) 
    {
        if (shared_mem_ptr->destination_floor[0] == '1') 
        {
            shared_mem_ptr->destination_floor[0] = 'B'; 
            shared_mem_ptr->destination_floor[1] = '1';
        } 
        else 
        {
            int number = atoi(&shared_mem_ptr->destination_floor[0]);
            number--;
            snprintf(&shared_mem_ptr->destination_floor[0], 12, "%d", number); 
        }
    }
    pthread_cond_signal(&shared_mem_ptr->cond);  
    pthread_mutex_unlock(&shared_mem_ptr->mutex);
}

int main(int argc, char *argv[]) 
{

    if (argc != 3) {
        fprintf(stderr, "Usage: %s {car_name} {opperation}\n", argv[0]);
        exit(1);
    }

    snprintf(shm_name, sizeof(shm_name), "/car%s", argv[1]); 

    shm_fd = shm_open(shm_name, O_RDWR, 0666);  // Not creating, just looking for shared memory
    if (shm_fd == -1) 
    {
        if (errno == ENOENT)    // ENOENT is the error given by no shared memory
        {
            printf("Unable to access car %s.\n", argv[1]);
            exit(1);
        }
        else
        {
            perror("shm_open failed\n");
            exit(1);
        }
    }

    if (ftruncate(shm_fd, sizeof(car_shared_mem)) == -1) {  // Resizing the memory from 0 bytes to the size needed for the entire typedef struc
        perror("ftruncate failed\n");
        exit(1);
    }

    // NULL: Location of mapped memory anywhere. sizeof: Length of memory mapped. READ|WRITE: Allow access to this. SHARED: Shared between processes. shm_fd: Object being mapped. 0: Offset of location
    shared_mem_ptr = mmap(NULL, sizeof(car_shared_mem), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // Allows other code to access and modify this info
    if (shared_mem_ptr == MAP_FAILED) {
        perror("mmap failed\n");
        exit(1);
    }
    // Going through all of the options for inputs in internal and doing the corresponding job
    if (strcmp(argv[2], "open") == 0) 
    {
        press_open_button(shared_mem_ptr);
    } 
    else if (strcmp(argv[2], "close") == 0) 
    {
        press_close_button(shared_mem_ptr);
    } 
    else if (strcmp(argv[2], "stop") == 0) 
    {
        press_stop_button(shared_mem_ptr);
    } 
    else if (strcmp(argv[2], "service_on") == 0) 
    {
        service_on(shared_mem_ptr);
    } 
    else if (strcmp(argv[2], "service_off") == 0) 
    {
        service_off(shared_mem_ptr);
    } 
    else if (strcmp(argv[2], "up") == 0)  
    {
        if (strcmp(shared_mem_ptr->status, "Between") == 0)  // elevator is moving between floors
        {
            printf("Operation not allowed while elevator is moving.\n");
        }
        else if (strcmp(shared_mem_ptr->status, "Closed") != 0 && strcmp(shared_mem_ptr->status, "Between") !=0)    // doors are open when called
        {
            printf("Operation not allowed while doors are open.\n");
        }
        else if (shared_mem_ptr->individual_service_mode != 1)   // called but not in service
        {
            printf("Operation only allowed in service mode.\n");
        }
        else
        {
            move_up(shared_mem_ptr);
        }
    } 
    else if (strcmp(argv[2], "down") == 0) 
    {
        if (strcmp(shared_mem_ptr->status, "Between") == 0)  // elevator is moving between floors
        {
            printf("Operation not allowed while elevator is moving.\n");
        }
        else if (strcmp(shared_mem_ptr->status, "Closed") != 0 && strcmp(shared_mem_ptr->status, "Between") !=0)    // doors are open when called
        {
            printf("Operation not allowed while doors are open.\n");
        }
        else if (shared_mem_ptr->individual_service_mode != 1)   // called but not in service
        {
            printf("Operation only allowed in service mode.\n");
        }
        else
        {
            move_down(shared_mem_ptr);
        }
    } 
    else 
    {
        printf("Invalid operation.\n");
    }

    return 0;
}
