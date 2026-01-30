#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

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
car_shared_mem *shared_mem_ptr; // Making a pointer for the car_shared_memory typedef struc
char shm_name[256];
int sockfd = -1; // Global variable for socket file descriptor
int connected = 0;
pthread_t tcp_thread;
pthread_t status_change;

void interupt_signal(int sig)
{
    if (munmap(shared_mem_ptr, sizeof(car_shared_mem)) == -1) {
        perror("munmap failed");
        exit(1);
    }

    if (close(shm_fd) == -1) {
        perror("close failed");
        exit(1);
    }

    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink failed");
    }
    exit(0);
}

int connect_to_controller() 
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket() failed");
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3000);
    const char *ipaddress = "127.0.0.1";
    if (inet_pton(AF_INET, ipaddress, &addr.sin_addr) != 1) {
        fprintf(stderr, "inet_pton(%s) failed\n", ipaddress);
        close(sockfd);
        return -1;
    }
    if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) == -1) 
    {
        perror("Failed to connect to controller");
        close(sockfd);
        return -1;
    }
    return 0;
}

void send_looped(int fd, const void *buf, size_t sz)
{
    const char *ptr = buf;
    size_t remain = sz;
    while (remain > 0) 
    {
        ssize_t sent = write(fd, ptr, remain);
        if (sent == -1) 
        {
            perror("write()");
            exit(1);
        }
        ptr += sent;
        remain -= sent;
    }
}

void send_message(int fd, const char *buf)
{
    uint32_t len = htonl(strlen(buf));
    send_looped(fd, &len, sizeof(len));
    send_looped(fd, buf, strlen(buf));
}

void *tcp_connection_handler(void *arg) 
{
    char **argv = (char **)arg;
    char buffer[256];

    while (1) {
        if (!connected) {
            if (connect_to_controller() == 0) 
            {
                connected = 1;

                snprintf(buffer, sizeof(buffer), "CAR {%s} {%s} {%s}", argv[1], argv[2], argv[3]);
                send_message(sockfd, buffer);
            }
        }

        if (connected) 
        {
            int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                printf("Received from controller: %s\n", buffer);
            } 
            else 
            {
                if (bytes_received == 0) 
                {
                    printf("Controller disconnected.\n");
                } 
                else 
                {
                    perror("recv() failed");
                }
                connected = 0;
                close(sockfd);
                sockfd = -1;
            }
        }
        sleep(1);
    }
    return NULL;
}

void *car_movement(void *arg)
{
    char **argv = (char **)arg;
    int delay = atoi(argv[4]);
    while(1) 
    {
        if (strcmp(shared_mem_ptr->current_floor, shared_mem_ptr->destination_floor) != 0 && (strcmp(shared_mem_ptr->status, "Closed") == 0 || strcmp(shared_mem_ptr->status, "Between") == 0))
        {
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            strncpy(shared_mem_ptr->status, "Between", sizeof(shared_mem_ptr->status));
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
            usleep(delay * 1000);           
            int current_floor_val = atoi(shared_mem_ptr->current_floor ); 
            int destination_floor_val = atoi(shared_mem_ptr->destination_floor);
            int floors_left = 0;
            // printf("Values: %d\n", current_floor_val);
            // printf("Values: %d\n", destination_floor_val);
            if ((shared_mem_ptr->current_floor[0] == 'B'&& shared_mem_ptr->destination_floor[0] == 'B') || (isalpha(shared_mem_ptr->current_floor[0]) && isalpha(shared_mem_ptr->destination_floor[0])))
            {
                floors_left = abs(current_floor_val - destination_floor_val);
                printf("floors left: %d\n", floors_left);
            }
            else if (shared_mem_ptr->current_floor[0] == 'B' && isalpha(shared_mem_ptr->destination_floor[0]))
            {
                floors_left = abs(current_floor_val + destination_floor_val - 1);  
                printf("floors left: %d\n", floors_left); 
            }
            for (int i = 0; i < floors_left; i++)
            {
                if (current_floor_val < destination_floor_val)
                {
                    current_floor_val++;
                }
                else
                {
                    current_floor_val--;
                }
                char new_floor[10];
                snprintf(new_floor, sizeof(new_floor), "B%d", current_floor_val);
                pthread_mutex_lock(&shared_mem_ptr->mutex);
                strncpy(shared_mem_ptr->current_floor, new_floor, sizeof(shared_mem_ptr->current_floor));
                pthread_cond_signal(&shared_mem_ptr->cond);  
                pthread_mutex_unlock(&shared_mem_ptr->mutex);
            }
        }
        if (argv[2] == argv[3] && strcmp(shared_mem_ptr->status, "Between") == 0)
        {
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            strncpy(shared_mem_ptr->status, "Opening", sizeof(shared_mem_ptr->status));
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
            usleep(delay*1000);
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            strncpy(shared_mem_ptr->status, "Open", sizeof(shared_mem_ptr->status));
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
            usleep(delay*1000);
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            strncpy(shared_mem_ptr->status, "Closing", sizeof(shared_mem_ptr->status));
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
            usleep(delay*1000);
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            strncpy(shared_mem_ptr->status, "Closed", sizeof(shared_mem_ptr->status));
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
        }
        if(shared_mem_ptr->open_button == 1 && (strcmp(shared_mem_ptr->status, "Closed") == 0 || strcmp(shared_mem_ptr->status, "Closing") == 0))  
        {
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            shared_mem_ptr->open_button = 0;
            strncpy(shared_mem_ptr->status, "Opening", sizeof(shared_mem_ptr->status));
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
            usleep(delay*1000);
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            strncpy(shared_mem_ptr->status, "Open", sizeof(shared_mem_ptr->status));
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
        }
        else if(shared_mem_ptr->close_button == 1 && (strcmp(shared_mem_ptr->status, "Open") == 0 || strcmp(shared_mem_ptr->status, "Opening") == 0))
        {
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            shared_mem_ptr->close_button = 0;
            strncpy(shared_mem_ptr->status, "Closing", sizeof(shared_mem_ptr->status));
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
            usleep(delay*1000);
            pthread_mutex_lock(&shared_mem_ptr->mutex);
            strncpy(shared_mem_ptr->status, "Closed", sizeof(shared_mem_ptr->status));
            pthread_cond_signal(&shared_mem_ptr->cond);  
            pthread_mutex_unlock(&shared_mem_ptr->mutex);
        }

    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) 
    {
        fprintf(stderr, "Usage: %s {name} {lowest floor} {highest floor} {delay}\n", argv[0]);
        exit(1);
    }
    snprintf(shm_name, sizeof(shm_name), "/car%s", argv[1]);    // Reshaping the name with car and storing it into the shm_name buffer
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);    // Making a POSIX shared memory allocation for access between processes
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(1);
    }
    if (ftruncate(shm_fd, sizeof(car_shared_mem)) == -1) // Resizing the memory from 0 bytes to the size needed for the entire typedef struc
    {  
        perror("ftruncate failed");
        exit(1);
    }

    // NULL: Location of mapped memory anywhere. sizeof: Length of memory mapped. READ|WRITE: Allow access to this. SHARED: Shared between processes. shm_fd: Object being mapped. 0: Offset of location
    shared_mem_ptr = mmap(NULL, sizeof(car_shared_mem) + 4, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // Allows other code to access and modify this info
    if (shared_mem_ptr == MAP_FAILED) 
    {
        perror("mmap failed");
        exit(1);
    }

    pthread_mutexattr_t mutex_attr;  // Declaring mutex attributes
    pthread_condattr_t cond_attr;   // Setting a state that can let other processes know when the mutex is unlocked to save constant checking

    pthread_mutexattr_init(&mutex_attr); // Initilising mutex attributes
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);   // Making the mutex shared between processes
    pthread_mutex_init(&shared_mem_ptr->mutex, &mutex_attr); // Mutex initilised with respect to shared 

    pthread_condattr_init(&cond_attr);  // Initilising  cond attributes
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);    // Shared
    pthread_cond_init(&shared_mem_ptr->cond, &cond_attr);   // Initilised with shared

    pthread_mutex_lock(&shared_mem_ptr->mutex); // Locking the mutex
    strncpy(shared_mem_ptr->current_floor, argv[2], 4);  // Start at the lowest floor
    strncpy(shared_mem_ptr->destination_floor, argv[2], 4); // Destination is the same initially
    strncpy(shared_mem_ptr->status, "Closed", sizeof(shared_mem_ptr->status));  // Initial status
    shared_mem_ptr->open_button = 0;
    shared_mem_ptr->close_button = 0;
    shared_mem_ptr->door_obstruction = 0;
    shared_mem_ptr->overload = 0;
    shared_mem_ptr->emergency_stop = 0;
    shared_mem_ptr->individual_service_mode = 0;
    shared_mem_ptr->emergency_mode = 0;
    pthread_mutex_unlock(&shared_mem_ptr->mutex);

    //printf("Current state: {%s, %s, %s, %d, %d, %d, %d, %d, %d, %d}\n", shared_mem_ptr->current_floor, shared_mem_ptr->destination_floor, shared_mem_ptr->status, shared_mem_ptr->open_button, shared_mem_ptr->close_button, shared_mem_ptr->door_obstruction, shared_mem_ptr->overload, shared_mem_ptr->emergency_stop, shared_mem_ptr->individual_service_mode, shared_mem_ptr->emergency_mode);
    signal(SIGINT, interupt_signal); // Setting the interupt funcion to run when ctrl + c are pressed (munmap)
    // if (pthread_create(&tcp_thread, NULL, tcp_connection_handler, argv) != 0)
    // {
    //     perror("Failed to create TCP thread");
    //     return 1;
    // }
    if (pthread_create(&status_change, NULL, car_movement, argv) != 0)
    {
        perror("Failed to create TCP thread");
        return 1;
    }
    
    // pthread_join(tcp_thread, NULL);
    pthread_join(status_change, NULL);
    return 0;
}