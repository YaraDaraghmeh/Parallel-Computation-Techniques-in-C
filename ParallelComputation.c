#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define N 12113513  // Define size of packets
#define M 8         // Define number of processes/threads

double packet1[N];
double packet2[N];
double result_packet1[N];
double *result_packet2;  // Shared memory pointer for parallel (shared memory)
double result_packet3[N]; // Array for message passing results
double result_packet4[N]; // Array for multithreading results

void initialize_packets() {
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        packet1[i] = (double)rand() / (double)(RAND_MAX / N);
        packet2[i] = (double)rand() / (double)(RAND_MAX / 10);
    }
}

void serial_calculation() {
    clock_t start = clock();
    for (int i = 0; i < N; i++) {
        result_packet1[i] = pow(packet1[i], packet2[i]);
    }
    clock_t end = clock();
    printf("Serial calculation time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
}

void parallel_calculation_with_shared_memory() {
    int segment_size = N / M;
    pid_t pids[M];
    int remainder = N % M;

    clock_t start = clock();
    result_packet2 = mmap(NULL, N * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (int i = 0; i < M; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {  // Child process
            int start_idx = i * segment_size;
            int end_idx = (i == M - 1) ? (start_idx + segment_size + remainder) : (start_idx + segment_size);
            for (int j = start_idx; j < end_idx; j++) {
                result_packet2[j] = pow(packet1[j], packet2[j]);
            }
            _exit(0);  // Exit child process
        }
    }

    for (int i = 0; i < M; i++) {
        waitpid(pids[i], NULL, 0);  // Wait for all child processes to complete
    }

    clock_t end = clock();
    printf("Parallel (shared memory) calculation time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Verify result_packet1 == result_packet2
    int matched = 1;
    for (int i = 0; i < N; i++) {
        if (fabs(result_packet1[i] - result_packet2[i]) > 1e-6) {
            printf("Mismatch at index %d in shared memory calculation\n", i);
            matched = 0;
            break;
        }
    }
    if (matched) {
        printf("Results match between serial and shared memory calculation.\n");
    }

    munmap(result_packet2, N * sizeof(double));
}

void parallel_calculation_with_message_passing() {
    int segment_size = N / M;
    pid_t pids[M];
    int remainder = N % M;
    char fifo_name[20];

    clock_t start = clock();

    for (int i = 0; i < M; i++) {
        snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_%d", i);
        mkfifo(fifo_name, 0666);
        pids[i] = fork();
        if (pids[i] == 0) {
            int start_idx = i * segment_size;
            int end_idx = (i == M - 1) ? (start_idx + segment_size + remainder) : (start_idx + segment_size);
            int fd = open(fifo_name, O_WRONLY);
            for (int j = start_idx; j < end_idx; j++) {
                double result = pow(packet1[j], packet2[j]);
                write(fd, &result, sizeof(double));
            }
            close(fd);
            _exit(0);
        }
    }

    for (int i = 0; i < M; i++) {
        snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_%d", i);
        int fd = open(fifo_name, O_RDONLY);
        for (int j = i * segment_size; j < (i + 1) * segment_size + (i == M - 1 ? remainder : 0); j++) {
            read(fd, &result_packet3[j], sizeof(double));
        }
        close(fd);
        unlink(fifo_name);
        waitpid(pids[i], NULL, 0);
    }

    clock_t end = clock();
    printf("Parallel (message passing) calculation time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    int matched = 1;
    for (int i = 0; i < N; i++) {
        if (fabs(result_packet1[i] - result_packet3[i]) > 1e-6) {
            printf("Mismatch at index %d in message passing calculation\n", i);
            matched = 0;
            break;
        }
    }
    if (matched) {
        printf("Results match between serial and message passing calculation.\n");
    }
}

typedef struct {
    int start_idx;
    int end_idx;
} ThreadData;

void* thread_calculation(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int i = data->start_idx; i < data->end_idx; i++) {
        result_packet4[i] = pow(packet1[i], packet2[i]);
    }
    pthread_exit(NULL);
}

void parallel_calculation_with_threads() {
    pthread_t threads[M];
    ThreadData thread_data[M];
    int segment_size = N / M;
    int remainder = N % M;

    clock_t start = clock();

    for (int i = 0; i < M; i++) {
        thread_data[i].start_idx = i * segment_size;
        thread_data[i].end_idx = (i == M - 1) ? (thread_data[i].start_idx + segment_size + remainder) : (thread_data[i].start_idx + segment_size);
        pthread_create(&threads[i], NULL, thread_calculation, &thread_data[i]);
    }

    for (int i = 0; i < M; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t end = clock();
    printf("Parallel (threads) calculation time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    int matched = 1;
    for (int i = 0; i < N; i++) {
        if (fabs(result_packet1[i] - result_packet4[i]) > 1e-6) {
            printf("Mismatch at index %d in threads calculation\n", i);
            matched = 0;
            break;
        }
    }
    if (matched) {
        printf("Results match between serial and threads calculation.\n");
    }
}

int main() {
    initialize_packets();

    printf("Starting serial calculation...\n");
    serial_calculation();

    printf("Starting parallel calculation with shared memory...\n");
    parallel_calculation_with_shared_memory();

    printf("Starting parallel calculation with message passing...\n");
    parallel_calculation_with_message_passing();

    printf("Starting parallel calculation with threads...\n");
    parallel_calculation_with_threads();

    return 0;
}
