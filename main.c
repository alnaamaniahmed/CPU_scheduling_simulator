#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define TASK_NAME_LENGTH 4
#define MAX_TASKS 50
#define BUFFER_SIZE 512
#define TIME_QUANTUM 4

typedef struct {
    char name[TASK_NAME_LENGTH];
    int arrivalTime;
    int burstTime;
    int waitingTime;
} Tasks;
typedef struct Node {
    Tasks task;
    struct Node *next;
} Node;

// Function to add a task to the queue
void enqueue(Node **head, Tasks task) {
    Node *newNode = (Node *) malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    newNode->task = task;
    newNode->next = NULL;
    if (*head == NULL) {
        *head = newNode;
    } else {
        Node *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}
// Function to remove and return the first task from the queue
Tasks dequeue(Node **head) {
    if (*head == NULL) {
        Tasks emptyTask;
        memset(&emptyTask, 0, sizeof(emptyTask));
        return emptyTask;
    }
    Node *temp = *head;
    Tasks task = temp->task;
    *head = temp->next;
    free(temp);
    return task;
}
// Function to calculate waiting time for each task in FCFS scheduling
void calculate_waiting_time(Tasks task[], int arrIndex) {
    task[0].waitingTime = 0;
    int timePassed = task[0].burstTime;
    for (int i = 1; i < arrIndex; i++) {
        task[i].waitingTime = timePassed - task[i].arrivalTime;
        timePassed = timePassed + task[i].burstTime;
    }
}
// Function to perform FCFS scheduling
void fcfs_scheduling(Tasks task[], int arrIndex, FILE *output) {
    calculate_waiting_time(task, arrIndex);

    fprintf(output, "FCFS:\n");

    int currentTime = 0;
    for (int i = 0; i < arrIndex; i++) {
        int startTime = currentTime;
        int finishTime = startTime + task[i].burstTime;
        fprintf(output, "%s %d %d\n", task[i].name, startTime, finishTime);
        currentTime = finishTime;
    }

    float sumWaitingTime = 0;
    for (int i = 0; i < arrIndex; i++) {
        fprintf(output, "Waiting Time %s: %d\n", task[i].name, task[i].waitingTime);
        sumWaitingTime += task[i].waitingTime;
    }
    float averageWaitingTime = sumWaitingTime / (float) arrIndex;
    fprintf(output, "Average Waiting Time: %.2f\n\n", averageWaitingTime);
}
// Function to perform Round-Robin scheduling
void rr_scheduling(Tasks task[], int arrIndex, FILE *output) {
    Node *queue = NULL;
    int currentTime = 0;
    int sumWaitingTime = 0;
    int taskIndex = 0;
    int *remainingTime = (int *) malloc(arrIndex * sizeof(int));
    int *lastFinishTime = (int *) malloc(arrIndex * sizeof(int));
    if (!remainingTime || !lastFinishTime) {
        fprintf(stderr, "Memory allocation failed\n");
        free(remainingTime);
        free(lastFinishTime);
        return;
    }

    // Initialize tasks and enqueue initial tasks
    for (int i = 0; i < arrIndex; i++) {
        remainingTime[i] = task[i].burstTime;
        lastFinishTime[i] = task[i].arrivalTime;
    }
    while (taskIndex < arrIndex && task[taskIndex].arrivalTime == 0) {
        enqueue(&queue, task[taskIndex++]);
    }


    fprintf(output, "RR:\n");

    // Initial enqueue of tasks that have arrived
    while (queue != NULL) {
        Tasks currentTask = dequeue(&queue);
        int index = currentTask.name[1] - '1';

        int execTime = (remainingTime[index] > TIME_QUANTUM) ? TIME_QUANTUM : remainingTime[index];
        fprintf(output, "%s %d %d\n", currentTask.name, currentTime, currentTime + execTime);

        currentTime += execTime;
        remainingTime[index] -= execTime;

        sumWaitingTime += currentTime - lastFinishTime[index] - execTime;

        lastFinishTime[index] = currentTime;

        // Check for new arrivals or re-enqueue the current task if it's not finished
        while (taskIndex < arrIndex && task[taskIndex].arrivalTime <= currentTime) {
            enqueue(&queue, task[taskIndex]);
            taskIndex++;
        }
        if (remainingTime[index] > 0) {
            enqueue(&queue, currentTask);
        }

        // If the queue is empty but there are still tasks to enqueue
        if (queue == NULL && taskIndex < arrIndex) {
            currentTime = task[taskIndex].arrivalTime;
            enqueue(&queue, task[taskIndex]);
            taskIndex++;
        }
    }

    // Calculate and print waiting times for each task
    for (int i = 0; i < arrIndex; i++) {
        int waitingTime = lastFinishTime[i] - task[i].arrivalTime - task[i].burstTime;
        fprintf(output, "Waiting Time %s: %d\n", task[i].name, waitingTime);
    }

    float averageWaitingTime = (float) sumWaitingTime / arrIndex;
    // Calculate the average waiting time
    fprintf(output, "Average Waiting Time: %.2f\n\n", averageWaitingTime);
    free(remainingTime);
    free(lastFinishTime);
}
// Function to perform Non-Preemptive Shortest Job First scheduling
void nsjf_scheduling(Tasks task[], int arrIndex, FILE *output) {
    int currentTime = 0;
    float sumWaitingTime = 0;
    int completedTasks = 0;
    // Array to keep track of completed tasks
    int taskDone[MAX_TASKS] = {0};

    fprintf(output, "NSJF:\n");

    while (completedTasks < arrIndex) {
        int shortestIndex = -1;
        int shortestBurst = INT_MAX;

        for (int i = 0; i < arrIndex; i++) {
            if (task[i].arrivalTime <= currentTime && task[i].burstTime < shortestBurst && !taskDone[i]) {
                shortestBurst = task[i].burstTime;
                shortestIndex = i;
            }
        }

        if (shortestIndex != -1) {
            Tasks *selectedTask = &task[shortestIndex];
            selectedTask->waitingTime = currentTime - selectedTask->arrivalTime;
            sumWaitingTime += selectedTask->waitingTime;

            int startTime = currentTime;
            int finishTime = startTime + selectedTask->burstTime;

            fprintf(output, "%s %d %d\n", selectedTask->name, startTime, finishTime);
            currentTime = finishTime;
            taskDone[shortestIndex] = 1; // Mark task as completed
            completedTasks++;
        } else {
            currentTime++; // Advance time if no suitable task is found
        }
    }

    for (int i = 0; i < arrIndex; i++) {
        fprintf(output, "Waiting Time %s: %d\n", task[i].name, task[i].waitingTime);
    }
    float averageWaitingTime = sumWaitingTime / arrIndex;
    fprintf(output, "Average Waiting Time: %.2f\n\n", averageWaitingTime);
}

int completedTasks(int n, const int isCompleted[]) {
    int completed = 0;
    for (int i = 0; i < n; i++) {
        if (isCompleted[i]) {
            completed++;
        }
    }
    return completed;
}
// Function to perform Preemptive Shortest Job First scheduling
void psjf_scheduling(Tasks task[], int arrIndex, FILE *output) {
    int currentTime = 0;
    int totalWaitingTime = 0;
    // Track the total executed time of tasks
    int executedTime[arrIndex];
    // Track the remaining execution time of tasks
    int remainingTime[arrIndex];
    // Track completion of tasks
    int isCompleted[arrIndex];

    memset(executedTime, 0, sizeof(executedTime));
    memset(isCompleted, 0, sizeof(isCompleted));
    for (int i = 0; i < arrIndex; i++) {
        remainingTime[i] = task[i].burstTime;
    }

    fprintf(output, "PSJF:\n");

    while (completedTasks(arrIndex, isCompleted) < arrIndex) {
        int minTime = INT_MAX;
        int shortestIndex = -1;
        for (int i = 0; i < arrIndex; i++) {
            if (task[i].arrivalTime <= currentTime && !isCompleted[i] && remainingTime[i] < minTime) {
                minTime = remainingTime[i];
                shortestIndex = i;
            }
        }

        if (shortestIndex != -1) {
            remainingTime[shortestIndex]--;
            executedTime[shortestIndex]++;
            if (remainingTime[shortestIndex] == 0) {
                isCompleted[shortestIndex] = 1;
                task[shortestIndex].waitingTime = currentTime + 1 - task[shortestIndex].burstTime - task[shortestIndex].arrivalTime;
                totalWaitingTime += task[shortestIndex].waitingTime;
            }
        }
        currentTime++;
    }

    for (int i = 0; i < arrIndex; i++) {
        fprintf(output, "Waiting Time %s: %d\n", task[i].name, task[i].waitingTime);
    }

    float averageWaitingTime = (float) totalWaitingTime / arrIndex;
    fprintf(output, "Average Waiting Time: %.2f\n\n", averageWaitingTime);
}


int main() {
    FILE *fp = fopen("TaskSpec.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }
    char buffer[BUFFER_SIZE];
    Tasks task[MAX_TASKS];
    int arrIndex = 0;
    while (fgets(buffer, BUFFER_SIZE, fp) && arrIndex < MAX_TASKS) {
        if (sscanf(buffer, "%3[^,],%d,%d", task[arrIndex].name, &task[arrIndex].arrivalTime, &task[arrIndex].burstTime) != 3) {
            fprintf(stderr, "File format incorrect\n");
            fclose(fp);
            return 1;
        }
        arrIndex++;
    }
    fclose(fp);

    FILE *output = fopen("Output.txt", "w");
    if (output == NULL) {
        fprintf(stderr, "Error opening output file\n");
        return 1;
    }

    fcfs_scheduling(task, arrIndex, output);
    rr_scheduling(task, arrIndex, output);
    nsjf_scheduling(task, arrIndex, output);
    psjf_scheduling(task, arrIndex, output);

    fclose(output);


    return 0;
}
