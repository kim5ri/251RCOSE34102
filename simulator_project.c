#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PROCESS_NUM 7
#define MAX_ARRIVAL 20
#define MAX_CPU_BURST 20
#define MAX_IO_BURST 5
#define MAX_PRIORITY 7
#define MAX_TIME_QUANTUM 5

// 프로세스 구조체 정의
typedef struct process{
    int pid;
    int CPU_burst;
    int IO_burst;
    int IO_request;
    int arrival;
    int priority;
    int CPU_remaining;
    int IO_remaining;
    int time_quntum;
    int waiting_time;
    int turnaround_time;
}process;

// ready queue, waiting queue를 위한 배열 구조체
typedef struct queue{
    process p[MAX_PROCESS_NUM + 1]; // 프로세스 구조체 배열
    int front;
    int rear;
    int size;
}queue;

typedef struct gantt_chart{
    int chart[400];
    int count;
}gantt;

process done[MAX_PROCESS_NUM];
int done_count = 0;



// 프로세스 및 큐 관련 함수 선언
queue *create_queue(void);
void enqueue(queue *q, process *p);
void dequeue(queue *q);
void create_process(queue *job_queue);
void config(queue **ready_queue, queue **waiting_queue, queue **job_queue, gantt **gantt_chart);
void io_execute(queue *waiting_queue, queue *ready_queue);

void print_gantt_chart(gantt *gantt_chart);
void save_gantt_chart(process *p, gantt *gantt_chart);

void schedualer_FCFS(queue *rq, queue *wq, queue *jq, gantt *gc);
void schedualer_NP_SJF(queue *rq, queue *wq, queue *jq, gantt *gc);
void schedualer_SJF(queue *rq, queue *wq, queue *jq, gantt *gc);
void schedualer_NP_priority(queue *rq, queue *wq, queue *jq, gantt *gc);
void schedualer_priority(queue *rq, queue *wq, queue *jq, gantt *gc);
void schedualer_RR(queue *rq, queue *wq, queue *jq, gantt *gc);

void evaluation(void);
void copy_queue(queue *new, queue *og);



// 큐 생성성
queue *create_queue(void){
    queue *newqueue = malloc(sizeof(queue));
    if(newqueue == NULL) return NULL;

    newqueue->front = 0;
    newqueue->rear = -1;
    newqueue->size = 0;
    return newqueue;
}

// 큐에 enqueue
void enqueue(queue *q, process *p){
    if(q->size >= MAX_PROCESS_NUM) return;                  // 큐의 남은 공간 확인
    if(q->rear == MAX_PROCESS_NUM - 1) q->rear = -1;        // 배열 맨 끝이면 꼬리를 맨 앞으로 이동
    
    q->rear++;
    q->p[q->rear] = *p;
    q->size++;
}

// 큐에 dequeue
void dequeue(queue *q){
    if(q->size <= 0) return;                                // 큐가 비어있으면 리턴
    q->front++;
    q->size--;
    if(q->front == MAX_PROCESS_NUM) q->front = 0;           // 배열 맨 끝이면 머리를 맨 앞으로 이동
}

// 시스템 환경 설정(ready queue, waiting queue 생성)
void config(queue **ready_queue, queue **waiting_queue, queue **job_queue, gantt **gantt_chart){
    *ready_queue = create_queue();
    *waiting_queue = create_queue();
    *job_queue = create_queue();
    *gantt_chart = malloc(sizeof(gantt));
    
    // 간트차트 초기화
    for(int i = 0; i < 400; i++){
    (*gantt_chart)->chart[i] = 0;
    }

    (*gantt_chart)->count = 0;
}

// 프로세스 생성과 프로세스 데이터 입력
void create_process(queue *job_queue){
    int process_num = rand() % MAX_PROCESS_NUM + 1;         // 프로세스 개수 랜덤 생성
    int tq = rand() % MAX_TIME_QUANTUM + 1;         // time quantum 랜덤 생성

    for(int i = 0; i < process_num; i++){
        if(job_queue->size >= MAX_PROCESS_NUM) return;   // 큐의 남은 공간 확인
       
        process temp;
    
        // 프로세스를 구성하는 데이터 입력
        temp.pid = i + 1;                                 // 프로세스 ID
        temp.CPU_burst = rand() % MAX_CPU_BURST + 1;
        temp.IO_burst = rand() % MAX_IO_BURST + 1;
        temp.IO_request= rand() % (temp.CPU_burst);         // cpu의 max burst로 하면 안됨. 현재 할당된 cpu burst를 이용해 랜덤 부여
        if(temp.IO_request == 0) temp.IO_burst = 0;         // IO request가 없는 경우 burst도 0으로 설정
        temp.arrival = rand() % MAX_ARRIVAL;
        temp.priority = rand() % MAX_PRIORITY + 1;            // 우선순위는 1~10 이며 작을 수록 높은 우선순위
        temp.CPU_remaining = temp.CPU_burst;
        temp.IO_remaining = temp.IO_burst;
        temp.time_quntum = tq;
        temp.waiting_time = 0;
        temp.turnaround_time = 0;
        
        printf("Pid %d: CPU_burst = %d, IO_burst = %d, IO_request = %d, arrival = %d, priority = %d, time_quntum = %d\n", temp.pid, temp.CPU_burst, temp.IO_burst, temp.IO_request, temp.arrival, temp.priority, temp.time_quntum);

        enqueue(job_queue, &temp);                        // 프로세스를 ready queue에 추가
    } 
}

// waiting queue에 있는 프로세스의 IO 작업 수행
void io_execute(queue *waiting_queue, queue *ready_queue){
    int num = waiting_queue->size;

    for(int i = 0; i < num; i++){
        process temp = waiting_queue->p[waiting_queue->front];
        dequeue(waiting_queue);
        temp.IO_remaining--;

        if(temp.IO_remaining > 0){
            enqueue(waiting_queue, &temp);
        }
        else{
            enqueue(ready_queue, &temp);
        }
    }
}

// 저장한 간트 차트 출력
void print_gantt_chart(gantt *gantt_chart){
    printf("\n========== Gantt Chart ==========\n");
    for(int i = 0; i < gantt_chart->count; i++){
        if(gantt_chart->chart[i] == -1){
            printf("Idle|");
        }
        else{
            printf("P%d|", gantt_chart->chart[i]);
        }
        if((i+1) % 5 == 0){
            printf("   \t(%d)\n", i + 1);
        }
    }
}

// 간트차드 저장
void save_gantt_chart(process *p, gantt *gantt_chart){
    gantt_chart->chart[gantt_chart->count] = p->pid;
    gantt_chart->count++;
}

// 간트차트 저장(idle)
void save_gantt_chart_idle(gantt *gantt_chart){
    gantt_chart->chart[gantt_chart->count] = -1;
    gantt_chart->count++;
}

// arrival time에 따라 정렬
void sort_by_arrival(queue *q){
    for(int i = 0; i < q->size - 1; i++){
        for(int j = 0; j < q->size - 1 - i; j++){
            if(q->p[j].arrival > q->p[j+1].arrival){
                process temp = q->p[j];
                q->p[j] = q->p[j+1];
                q->p[j+1] = temp;
            }
        }
    }
}

// cpu remaining time이 가장 적은 프로세스를 맨 앞에 위치
void select_shortest(queue *q){
    int shortest = q->front;
    int index;

    // 가장 짧은 CPU_remaining을 가진 프로세스 찾기
    for(int i = 1; i < q->size; i++){
        index = (q->front + i) % (MAX_PROCESS_NUM);

        if(q->p[index].CPU_remaining < q->p[shortest].CPU_remaining){
            shortest = index;
        }
    }
    // shortest 위치랑 front 교환
   if(shortest != q->front){
        process temp = q->p[shortest];
        q->p[shortest] = q->p[q->front];
        q->p[q->front] = temp;
    }
}

// 우선순위가 가장 높은 프로세스를 맨 앞에 위치
void select_highest(queue *q){
    int highest = q->front;
    int index;

    // 가장 높은 우선순위를 가진 프로세스 찾기(숫자가 작을수록 높음)
    for(int i = 1; i < q->size; i++){
        index = (q->front + i) % (MAX_PROCESS_NUM);
        if(q->p[index].priority < q->p[highest].priority){
            highest = index;
        }
    }
    // highest 위치랑 front 교환
   if(highest != q->front){
        process temp = q->p[highest];
        q->p[highest] = q->p[q->front];
        q->p[q->front] = temp;
    }
}



// FCFS 스케줄링 알고리즘
void schedualer_FCFS(queue *rq, queue *wq, queue *jq, gantt *gc){
    int clock = 0;
    process *execute = NULL; // 현재 CPU에 올라와 있는 프로세스 포인터

    sort_by_arrival(jq);\
    
    while((jq->size > 0) || (rq->size > 0) || (wq->size > 0)){

        // clock = arrival인 프로세스는 모두 ready queue에 저장(job queue에서)
        while((jq->size > 0) && (jq->p[jq->front].arrival <= clock)){
            enqueue(rq, &jq->p[jq->front]);
            dequeue(jq);
        }
        // ready queue가 비어 있으면 idle상태
        if(rq->size == 0){
            save_gantt_chart_idle(gc);
            clock++;
            io_execute(wq, rq);
        }
        else{
            // ready queue의 front에 있는 프로세스 선택 후 dequeue
            execute = &rq->p[rq->front];
            dequeue(rq);

            // io_request가 있는 경우
            if(execute->IO_request > 0){
                while(execute->IO_request > 0){
                    save_gantt_chart(execute, gc);
                    execute->IO_request--;
                    execute->CPU_remaining--;
                    clock++;
                    
                    // cpu 실행 중에도 arrival된 프로세스는 ready queue에 저장
                    while((jq->size > 0) && (jq->p[jq->front].arrival <= clock)){
                        enqueue(rq, &jq->p[jq->front]);
                        dequeue(jq);
                    }
                    io_execute(wq, rq);
                }
                enqueue(wq, execute);
            }
            // io_request가 없는 경우
            else{
                while(execute->CPU_remaining > 0){
                    save_gantt_chart(execute, gc);
                    execute->CPU_remaining--;
                    clock++;
                    
                    // cpu 실행 중에도 arrival된 프로세스는 ready queue에 저장
                    while((jq->size > 0) && (jq->p[jq->front].arrival <= clock)){
                        enqueue(rq, &jq->p[jq->front]);
                        dequeue(jq);
                    }
                    io_execute(wq, rq);
                }
                execute->turnaround_time = clock - execute->arrival;
                execute->waiting_time = clock - execute->arrival - execute->CPU_burst - execute->IO_burst;
                done[done_count++] = *execute; // 완료된 프로세스 저장
            }
        }
    }
}

// Non-preemptive SJF 스케줄링 알고리즘즘
void schedualer_NP_SJF(queue *rq, queue *wq, queue *jq, gantt *gc){
    int clock = 0;
    process *execute = NULL; // 현재 CPU에 올라와 있는 프로세스 포인터

    sort_by_arrival(jq);
    
    while((jq->size > 0) || (rq->size > 0) || (wq->size > 0)){

        // clock = arrival인 프로세스는 모두 ready queue에 저장(job queue에서)
        while((jq->size > 0) && (jq->p[jq->front].arrival <= clock)){
            enqueue(rq, &jq->p[jq->front]);
            dequeue(jq);
        }

        // ready queue가 비어 있으면 idle
        if(rq->size == 0){
            save_gantt_chart_idle(gc);
            io_execute(wq, rq);
            clock++;
            continue;
        }
        // 가장 짧은 remaining time을 가진 프로세스를 추출
        select_shortest(rq);

        // ready queue의 front에 있는 프로세스 선택(shortest)
        execute = &rq->p[rq->front];
        dequeue(rq);

        // io_request가 있는 경우
        if(execute->IO_request > 0){
            while(execute->IO_request > 0){
                save_gantt_chart(execute, gc);
                execute->IO_request--;
                execute->CPU_remaining--;
                clock++;
                io_execute(wq, rq);
                
                // cpu 실행 중에도 arrival된 프로세스는 ready queue에 저장
                while((jq->size > 0) && (jq->p[jq->front].arrival <= clock)){
                    enqueue(rq, &jq->p[jq->front]);
                    dequeue(jq);
                }
            }
            enqueue(wq, execute);
        }
        // io_request가 없는 경우
        else{
            while(execute->CPU_remaining > 0){
                save_gantt_chart(execute, gc);
                execute->CPU_remaining--;
                clock++;
                io_execute(wq, rq);
                
                // cpu 실행 중에도 arrival된 프로세스는 ready queue에 저장
                while((jq->size > 0) && (jq->p[jq->front].arrival <= clock)){
                    enqueue(rq, &jq->p[jq->front]);
                    dequeue(jq);
                }             
            }
            // turnaround time과 waiting time 저장
            execute->turnaround_time = clock - execute->arrival;
            execute->waiting_time = clock - execute->arrival - execute->CPU_burst - execute->IO_burst;
            done[done_count++] = *execute; // 완료된 프로세스 저장
        }
    }
}

// preemptive SJF 스케줄링 알고리즘
void schedualer_SJF(queue *rq, queue *wq, queue *jq, gantt *gc)
{
    int clock = 0;
    int flag = 0;                      // io request 시간이 충족되면 io발생을 확인하기 위한 signal
    process *execute = NULL;
    
    sort_by_arrival(jq);

    while(jq->size > 0 || rq->size > 0 || wq->size > 0 || execute){

        // clock = arrival인 프로세스는 모두 ready queue에 저장(job queue에서)
        while(jq->size > 0 && jq->p[jq->front].arrival <= clock){
            enqueue(rq, &jq->p[jq->front]);
            dequeue(jq);
        }

        // 현재 실행 중인 프로세스보다 더 짧은 프로세스가 ready queue에 있으면 작업 교체
        if(execute && rq->size > 0){
            select_shortest(rq);
            if(rq->p[rq->front].CPU_remaining < execute->CPU_remaining){
                enqueue(rq, execute);
                execute = &rq->p[rq->front];
                dequeue(rq);
            }
        }

        // cpu가 비어있을 경우 ready queue에서 프로세스 선택(맨 앞이 가장 짧음), ready queue도 비어 있을 경우 idle 후 사이클 진행
        if(!execute){
            if(rq->size == 0){
                save_gantt_chart_idle(gc);
                io_execute(wq, rq);
                clock++;
                continue;
            }

            select_shortest(rq);
            execute = &rq->p[rq->front];
            dequeue(rq);
        }

        save_gantt_chart(execute, gc);
        execute->CPU_remaining--;

        // io request 시간 도달 시 io 작업 수행, flag 업데이트트
        if(execute->IO_request > 0){
            execute->IO_request--;
            flag = (execute->IO_request == 0); // io request가 0이 되면 falg를 1로 설정, 애초에 0으로 설정된 경우 조건문 분기 만족 x
        }
        else{
            flag = 0;
        }
        clock++;

        io_execute(wq, rq);

        while(jq->size > 0 && jq->p[jq->front].arrival <= clock){
            enqueue(rq, &jq->p[jq->front]);
            dequeue(jq);
        }

        // 프로세서 종료 및 io interrupt 시에 cpu에서 내림
        if(execute->CPU_remaining == 0){
            execute->turnaround_time = clock - execute->arrival;
            execute->waiting_time = clock - execute->arrival - execute->CPU_burst - execute->IO_burst;
            done[done_count++] = *execute; // 완료된 프로세스 저장
            execute = NULL;
        }
        else if(flag){
            execute->IO_request = -1;
            enqueue(wq, execute);
            execute = NULL;
        }
    }
}

// Non-preemptive priority 스케줄링 알고리즘
void schedualer_NP_priority(queue *rq, queue *wq, queue *jq, gantt *gc){
    int clock = 0;
    process *execute = NULL; // 현재 CPU에 올라와 있는 프로세스 포인터

    sort_by_arrival(jq);
    
    while((jq->size > 0) || (rq->size > 0) || (wq->size > 0)){

        // clock = arrival인 프로세스는 모두 ready queue에 저장(job queue에서)
        while((jq->size > 0) && (jq->p[jq->front].arrival <= clock)){
            enqueue(rq, &jq->p[jq->front]);
            dequeue(jq);
        }

        // ready queue가 비어 있으면 idle
        if(rq->size == 0){
            save_gantt_chart_idle(gc);
            io_execute(wq, rq);
            clock++;
            continue;
        }
        // 가장 높은 우선순위를 가진 프로세스를 추출
        select_highest(rq);

        // ready queue의 front에 있는 프로세스 선택(shortest)
        execute = &rq->p[rq->front];
        dequeue(rq);

        // io_request가 있는 경우
        if(execute->IO_request > 0){
            while(execute->IO_request > 0){
                save_gantt_chart(execute, gc);
                execute->IO_request--;
                execute->CPU_remaining--;
                clock++;
                io_execute(wq, rq);
                
                // cpu 실행 중에도 arrival된 프로세스는 ready queue에 저장
                while((jq->size > 0) && (jq->p[jq->front].arrival <= clock)){
                    enqueue(rq, &jq->p[jq->front]);
                    dequeue(jq);
                }
            }
            enqueue(wq, execute);
        }
        // io_request가 없는 경우
        else{
            while(execute->CPU_remaining > 0){
                save_gantt_chart(execute, gc);
                execute->CPU_remaining--;
                clock++;
                io_execute(wq, rq);
                
                // cpu 실행 중에도 arrival된 프로세스는 ready queue에 저장
                while((jq->size > 0) && (jq->p[jq->front].arrival <= clock)){
                    enqueue(rq, &jq->p[jq->front]);
                    dequeue(jq);
                }             
            }
            execute->turnaround_time = clock - execute->arrival;
            execute->waiting_time = clock - execute->arrival - execute->CPU_burst - execute->IO_burst;
            done[done_count++] = *execute; // 완료된 프로세스 저장
        }
    }
}

// Preemptive priority 스케줄링 알고리즘
void schedualer_priority(queue *rq, queue *wq, queue *jq, gantt *gc){
    int clock = 0;
    int flag = 0;                         // io request 시간이 충족되면 io발생을 확인하기 위한 signal
    process *execute = NULL;
    
    sort_by_arrival(jq);

    while(jq->size > 0 || rq->size > 0 || wq->size > 0 || execute){

        // clock = arrival인 프로세스는 모두 ready queue에 저장(job queue에서)
        while(jq->size > 0 && jq->p[jq->front].arrival <= clock){
            enqueue(rq, &jq->p[jq->front]);
            dequeue(jq);
        }

        // 현재 실행 중인 프로세스보다 우선순위가 높은 프로세스가 ready queue에 있으면 작업 교체
        if(execute && rq->size > 0){
            select_highest(rq);
            if(rq->p[rq->front].priority < execute->priority){
                enqueue(rq, execute);
                execute = &rq->p[rq->front];
                dequeue(rq);
            }
        }

        // cpu가 비어있을 경우 ready queue에서 프로세스 선택(맨 앞이 가장 우선순위가 높음), ready queue도 비어 있을 경우 idle
        if(!execute){
            if(rq->size == 0){
                save_gantt_chart_idle(gc);
                io_execute(wq, rq);
                clock++;
                continue;
            }

            select_highest(rq);
            execute = &rq->p[rq->front];
            dequeue(rq);
        }

        save_gantt_chart(execute, gc);
        execute->CPU_remaining--;
        if(execute->IO_request > 0){
            execute->IO_request--;
            flag = (execute->IO_request == 0); // io request가 0이 되면 falg를 1로 설정, 애초에 0으로 설정된 경우 조건문 분기 만족 x
        }
        else{
            flag = 0;
        }
        clock++;

        io_execute(wq, rq);

        while(jq->size > 0 && jq->p[jq->front].arrival <= clock){
            enqueue(rq, &jq->p[jq->front]);
            dequeue(jq);
        }
        
        // 프로세서 종료 및 io interrupt 시에 cpu에서 내림
        if(execute->CPU_remaining == 0){
            execute->turnaround_time = clock - execute->arrival;
            execute->waiting_time = clock - execute->arrival - execute->CPU_burst - execute->IO_burst;
            done[done_count++] = *execute; // 완료된 프로세스 저장
            execute = NULL;
        }
        else if(flag){
            execute->IO_request = -1;
            enqueue(wq, execute);
            execute = NULL;
        }
    }
}

void schedualer_RR(queue *rq, queue *wq, queue *jq, gantt *gc){
    int clock = 0;
    int quntum = 0;                     // time quantum
    int flag = 0;                       // io request가 있었는지 없었는지 구분분
    process *execute = NULL;

    sort_by_arrival(jq);

    
    while(jq->size > 0 || rq->size > 0 || wq->size > 0 || execute){
        // clock = arrival인 프로세스는 모두 ready queue에 저장(job queue에서)
        while((jq->size > 0) && (jq->p[jq->front].arrival <= clock)){
            enqueue(rq, &jq->p[jq->front]);
            dequeue(jq);
        }

        // cpu가 비어있을 경우 ready queue에서 프로세스 선택(RR 이므로 그냥 먼저 온 프로세스 선택), ready queue도 비어 있을 경우 idle
        if(!execute){
            if (rq->size == 0){
                io_execute(wq, rq);
                save_gantt_chart_idle(gc);
                clock++;
                continue;
            }

            execute = &rq->p[rq->front];
            dequeue(rq);
            quntum = execute->time_quntum;
            flag = 0;
        }

        save_gantt_chart(execute, gc);
        
        // io request가 0보다 큰 프로세서의 경우 io request 감소, 0이 되면 flag 1로 전환
        if(execute->IO_request > 0){
            execute->IO_request--;
            if(execute->IO_request == 0){
                flag = 1;
            }
        }

        // 사이클 진행 및 퀀텀 감소
        execute->CPU_remaining--;
        quntum--;
        clock++;
        io_execute(wq, rq);

        // job queue의 프로세서를 ready queue로(arrival time 만족)
        while(jq->size > 0 && jq->p[jq->front].arrival <= clock){
            enqueue(rq, &jq->p[jq->front]);
            dequeue(jq);
        }

        // 프로세서 종료, io interrupt, quntum 종료 시 cpu에서 프로세서 내림
        if(execute->CPU_remaining == 0){
            execute->turnaround_time = clock - execute->arrival;
            execute->waiting_time = clock - execute->arrival - execute->CPU_burst - execute->IO_burst;
            done[done_count++] = *execute; // 완료된 프로세스 저장
            execute = NULL;
        }
        else if(flag){
            execute->IO_request = -1;
            enqueue(wq, execute);
            execute = NULL;
        }
        else if(quntum == 0){
            enqueue(rq, execute);
            execute = NULL;
        }

        flag = 0;
    }
}

void evaluation(){
    double sum_w = 0, sum_t = 0; 
    printf("\n\n====== evaluation ======\n");
    printf("PID  wait  turn\n");
    for(int i = 0; i < done_count; i++){
        printf("%3d %5d %6d\n",
        done[i].pid,
        done[i].waiting_time,
        done[i].turnaround_time);
        sum_w += done[i].waiting_time;
        sum_t += done[i].turnaround_time;
    }
    printf("-----------------\n");
    printf("average waiting time : %.2f\n", sum_w / done_count);
    printf("average turnaround time : %.2f\n", sum_t / done_count);    
}

void copy_queue(queue *new, queue *og)
{
    memcpy(new, og, sizeof(queue));
}

//////////
// main //
//////////

#define FCFS 1
#define NP_SJF 2
#define SJF 3
#define NP_PR  4
#define PRIO   5
#define RR     6
#define QUIT   0


void main(){
    srand(time(NULL));

    queue *origin_job_queue;
    queue *ready_queue;
    queue *waiting_queue;
    gantt *gantt_chart;
    config(&ready_queue, &waiting_queue, &origin_job_queue, &gantt_chart);

    create_process(origin_job_queue);

    int select;

    do{
        puts("\n====== scheduler select ======");
        puts("1: FCFS");
        puts("2: Non-preemptive SJF");
        puts("3: Preemptive SJF");
        puts("4: Non-preemptive Priority");
        puts("5: Preemptive Priority");
        puts("6: Round-Robin");
        puts("0: Quit");
        printf("Select: ");

        scanf("\n%d", &select);

        if(select == QUIT) break;

        if(select < FCFS || select > RR){
            puts("invalid"); continue;
        }

        queue *job_queue = create_queue();
        waiting_queue->size   = 0;
        ready_queue->size  = 0;
        gantt_chart->count = 0;
        copy_queue(job_queue, origin_job_queue);

        switch(select){
            case FCFS: schedualer_FCFS(ready_queue, waiting_queue, job_queue, gantt_chart);
            break;

            case NP_SJF: schedualer_NP_SJF(ready_queue, waiting_queue, job_queue, gantt_chart);    
            break;

            case SJF: schedualer_SJF(ready_queue, waiting_queue, job_queue, gantt_chart);       
            break;

            case NP_PR: schedualer_NP_priority(ready_queue, waiting_queue, job_queue, gantt_chart); 
            break;

            case PRIO: schedualer_priority(ready_queue, waiting_queue, job_queue, gantt_chart);  
            break;

            case RR: schedualer_RR(ready_queue, waiting_queue, job_queue, gantt_chart);        
            break;
        }

        print_gantt_chart(gantt_chart);
        evaluation();

        free(job_queue);
        done_count = 0;
    } while(1);

    free(ready_queue);
    free(waiting_queue);
    free(origin_job_queue);
    free(gantt_chart);
}