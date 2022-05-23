//头文件区域/////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//宏定义区域/////////////////////////////////////////////
//名字的长度
#define SIZE 16
//定量时间
#define QUANTUM 4
#define READ_LINE_LEN 256

//结构体区域///////////////////////////////////////////////
/**
 * @brief 任务结构体
 *
 */
typedef struct
{
  //任务名字
  char name[SIZE];
  //优先级
  int priority;
  //爆发
  int burst;
  //到达时间
  int arrival_time;
  //调度总量
  int sched_sum;
  //等待时间
  int waiting_time;
  //开始时间
  int start_time;
  //结束时间
  int end_time;
} Task;

/**
 * @brief 节点结构体
 *
 */
struct Node
{
  //任务
  Task *task;
  //下一个节点
  struct Node *next;
} Node;

//变量区域///////////////////////////////////////////////
/**
 * @brief 文件指针
 *
 */
FILE *file = NULL;

/**
 * @brief 任务的链表头
 *
 */
struct Node *head_task;

/**
 * @brief 等待时间的链表头
 *
 */
struct Node *head_waiting_time;

/**
 * @brief 调度的链表头
 *
 */
struct Node *head_sched;

struct Node *cur_task_rr;

struct Node *running_node_psjf;

struct Node *ready_node_psjf;

//函数区域///////////////////////////////////////////////
/**
 * @brief 初始化链表头
 *
 */
void init_head()
{
  head_task = (struct Node *)malloc(sizeof(struct Node));
  head_task = NULL;

  head_waiting_time = (struct Node *)malloc(sizeof(struct Node));
  head_waiting_time = NULL;

  head_sched = (struct Node *)malloc(sizeof(struct Node));
  head_sched = NULL;
}

/**
 * @brief 打印任务结构体信息
 * 结构体每个成员都打印出来，调试时方便使用
 *
 * @param task 任务结构体
 */
void print_task(Task *task)
{
  printf("[%s]\n", task->name);
  printf("priority: %d\n", task->priority);
  printf("burst: %d\n", task->burst);
  printf("arrival_time: %d\n", task->arrival_time);
  printf("sched_sum: %d\n", task->sched_sum);
  printf("waiting_time: %d\n", task->waiting_time);
  printf("start_time: %d\n", task->start_time);
  printf("end_time: %d\n\n", task->end_time);
}

/**
 * @brief 插入一个新任务到链表中
 *
 * @param head 链表头
 * @param new_task 新任务
 */
void insert_node(struct Node **head, Task *new_task)
{
  struct Node *cur_node;
  struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
  new_node->task = new_task;
  new_node->next = NULL;

  cur_node = *head;
  if(cur_node == NULL)
  {
    *head = new_node;
    (*head)->next = NULL;
  } else {
    while(cur_node->next != NULL)
    {
      cur_node = cur_node->next;
    }
    cur_node->next = new_node;
  }
}

/**
 * @brief 从链接中删除一个任务
 * 根据任务名比较，相同则删除
 *
 * @param head 链表头
 * @param task 任务
 */
void delete_node(struct Node **head, Task *task)
{
  struct Node *cur_node = *head;
  struct Node *prev_node;

  while(cur_node != NULL && strcmp((cur_node->task)->name, task->name) != 0)
  {
    prev_node = cur_node;
    cur_node = cur_node->next;
  }
  if(cur_node == *head)
  {
    *head = cur_node->next;
    free(cur_node);
  } else if(cur_node == NULL)
  {
    printf("Delete node error: task not found.\n");
  } else
  {
    prev_node->next = cur_node->next;
    free(cur_node);
  }
}

/**
 * @brief 释放链表中所有节点的资源（动态分配的内存资源）
 *
 * @param head 链表头
 */
void free_all_node(struct Node *head)
{
  struct Node *tmp_node;
  struct Node *cur_node = head;
  
  while(cur_node != NULL)
  {
    tmp_node = cur_node->next;
    free(cur_node->task);
    free(cur_node);
    cur_node = tmp_node;
  }
}

/**
 * @brief 计算有几个节点
 *
 * @param head 节点头
 * @return int 返回节点个数
 */
int count_node(struct Node *head)
{
  int count = 0;
  struct Node *cur_node = head;
  while(cur_node != NULL)
  {
    count++;
    cur_node = cur_node->next;
  }

  return count;
}

/**
 * @brief 更新等待时间
 *
 * @param head 链表头
 * @param task 任务
 * @param slice 时间片
 */
void update_waiting_time_node(struct Node *head, Task *task, int slice)
{
  struct Node *cur_node = head;
  while(cur_node != NULL)
  {
    if((strcmp((cur_node->task)->name, task->name)) != 0)
    {
      (cur_node->task)->waiting_time += slice;
    }

    cur_node = cur_node->next;
  }
}

/**
 * @brief 打印等待时间
 *
 * @param head 链表头
 */
void print_waiting_time(struct Node *head)
{
  struct Node *cur_node = head;
  printf("Waiting Time:\n");
  while(cur_node != NULL)
  {
    printf("%s   %d\n", (cur_node->task)->name, (cur_node->task)->waiting_time);
    cur_node = cur_node->next;
  }
}

/**
 * @brief 打印调度列表
 *
 * @param head 链表头
 */
void print_sched_list(struct Node *head)
{
  struct Node *cur_node = head;
  printf("Scheduling:\n");
  while(cur_node != NULL)
  {
    printf("%s   %-5d%d\n", (cur_node->task)->name, (cur_node->task)->start_time, (cur_node->task)->end_time);
    cur_node = cur_node->next;
  }
}

/**
 * @brief 增加任务到链表中
 *
 * @param name 任务名字
 * @param priority 优先级
 * @param burst 爆发时间
 * @param arrival_time 到达时间
 */
void add_task(char *name, int priority, int burst, int arrival_time)
{
  Task *new_task = (Task *)malloc(sizeof(Task));
  strcpy(new_task->name, name);
  new_task->priority = priority;
  new_task->burst = burst;
  new_task->arrival_time = arrival_time;
  new_task->sched_sum = 0;
  new_task->waiting_time = 0 - arrival_time;

  insert_node(&head_task, new_task);
  insert_node(&head_waiting_time, new_task);
}

/**
 * @brief 比较前者是否小于后者
 *
 * @param src
 * @param dest
 * @return true
 * @return false
 */
bool min(int src, int dest) { return (src < dest); }

/**
 * @brief 比较两者是否相等
 *
 * @param src
 * @param dest
 * @return true
 * @return false
 */
bool equals(int src, int dest) { return (src == dest); }

/**
 * @brief 增加tid
 *
 * @param task
 */
void add_tid(Task *task) { task->sched_sum += 1; }

/**
 * @brief 减少时间片
 *
 * @param task
 * @param slice
 */
void sub_slice(Task *task, int slice) { task->burst -= slice; }

// FCFS

/**
 * @brief 取下一个任务（FCFS）
 *
 * @see delete_node
 *
 * @return Task*
 */
Task *pick_next_task_fcfs()
{
  Task *first_task;
  if(head_task == NULL)
  {
    first_task = NULL;
  } else
  {
    first_task = head_task->task;
    delete_node(&head_task, first_task);
  }

  return first_task;
}

/**
 * @brief 调度任务（FCFS）
 *
 * @see pick_next_task_fsfs
 * @see update_waiting_time_node
 * @see insert_node
 * @see print_sched_list
 * @see free_all_node
 * @see print_waiting_time
 *
 */
void schedule_task_fcfs()
{
  //调度链表记录开始时间和结束时间
  //等待时间链表记录等待时间：开始时间减去到达时间
  int count_task;
  float waiting_time_sum;
  float avg_waiting_time;
  Task *picked_task;
  int last_end_time = 0;

  //统计总任务数
  count_task = count_node(head_task);
  //更新调度链表和等待时间链表
  while((picked_task = pick_next_task_fcfs()) != NULL) {
    //任务链表按到达时间顺序排列，更新调度链表
    picked_task->start_time = last_end_time;
    picked_task->end_time = picked_task->start_time + picked_task->burst;
    last_end_time = picked_task->end_time;

    Task *new_sched_task = (Task *)malloc(sizeof(Task));
    *new_sched_task = *picked_task;
    insert_node(&head_sched, new_sched_task);
    
    //更新等待时间链表
    picked_task->waiting_time = picked_task->start_time - picked_task->arrival_time;

  }
  printf("[FCFS]\n");
  print_sched_list(head_sched);
  print_waiting_time(head_waiting_time);

  struct Node *cur_node = head_waiting_time;
  while(cur_node != NULL)
  {
    waiting_time_sum += (cur_node->task)->waiting_time;
    cur_node = cur_node->next;
  }
  avg_waiting_time = waiting_time_sum / count_task;
  printf("Average Waiting Time: %.2f\n", avg_waiting_time);
  
  //释放所有节点
  free_all_node(head_sched);
  free_all_node(head_waiting_time);
}

// RR

/**
 * @brief 获取定量时间 （RR）
 *
 * @param burst
 * @return int
 */
int get_slice_rr(int burst) { return ((burst < QUANTUM) ? burst : QUANTUM); }

/**
 * @brief 是否要删除任务（RR）
 *
 * @param burst
 * @return true
 * @return false
 */
bool is_delete_node_rr(int burst) { return (burst <= QUANTUM); }

/**
 * @brief 取下一个任务（RR）
 *
 * @see equals
 * @see add_tid
 * @see is_delete_node_rr
 * @see delete_node
 *
 * @return Task*
 */
Task *pick_next_task_rr()
{
  Task *picked_task;
  struct Node *deleted_node;
  //取任务链表当前任务
  if(cur_task_rr == NULL)
  {
    return NULL;
  } else
  {
    picked_task = cur_task_rr->task;
    add_tid(picked_task);
    if(cur_task_rr->next == NULL)
    {
      cur_task_rr = head_task;
    } else
    {
      cur_task_rr = cur_task_rr->next;
    }
    if(is_delete_node_rr(picked_task->burst))
    {
      if(cur_task_rr == head_task && cur_task_rr->next == NULL)
      {
        cur_task_rr = NULL;
      }
      delete_node(&head_task, picked_task);

      return picked_task;
    }
  }

  return picked_task;
}

/**
 * @brief 调度任务（RR）
 *
 * @see pick_next_task_rr
 * @see get_slice_rr
 * @see update_waiting_time_node
 * @see insert_node
 * @see is_delete_node_rr
 * @see sub_slice
 * @see print_sched_list
 * @see free_all_node
 * @see print_waiting_time
 *
 */
void schedule_task_rr()
{
  Task *picked_task;
  int count_task;
  float waiting_time_sum;
  float avg_waiting_time;
  int last_end_time = 0;
  
  count_task = count_node(head_task);

  cur_task_rr = head_task;
  while((picked_task = pick_next_task_rr()) != NULL)
  {
    int slice = get_slice_rr(picked_task->burst);
    Task *new_sched_task = (Task *)malloc(sizeof(Task));
    *new_sched_task = *picked_task;
    //更新调度链表
    new_sched_task->start_time = last_end_time;
    new_sched_task->end_time = new_sched_task->start_time + slice;
    last_end_time = new_sched_task->end_time;
    insert_node(&head_sched, new_sched_task);

    sub_slice(picked_task, slice);
    //更新等待时间链表
    update_waiting_time_node(head_task, picked_task, slice);
  }
  printf("\n[RR]\n");
  print_sched_list(head_sched);
  print_waiting_time(head_waiting_time);

  struct Node *cur_node = head_waiting_time;
  while(cur_node != NULL)
  {
    waiting_time_sum += (cur_node->task)->waiting_time;
    cur_node = cur_node->next;
  }
  avg_waiting_time = waiting_time_sum / count_task;
  printf("Average Waiting Time: %.2f\n", avg_waiting_time);
  
  //释放所有节点
  free_all_node(head_sched);
  free_all_node(head_waiting_time);
}

// NSJF

/**
 * @brief 取下一个任务（NSJF）
 *
 * @see min
 * @see equals
 * @see add_tid
 * @see delete_node
 *
 * @param time
 * @return Task*
 */
Task *pick_next_task_nsjf(int time)
{
  //参数time取当前时间
  //对在time时间以前到达的任务的执行时间进行比较，取运行时间最少的
  //每次取完一个，time参数都加上取出的任务的运行时间，取出的任务的节点删掉
  Task *picked_task;
  if(head_task == NULL)
  {
    return NULL;
  } else
  {
    picked_task = head_task->task;
  }
  struct Node *cur_node = head_task;
  while(cur_node != NULL && (cur_node->task)->arrival_time <= time)
  {
    if(min((cur_node->task)->burst, picked_task->burst))
    {
      picked_task = cur_node->task;
    }
    cur_node = cur_node->next;
  }
  delete_node(&head_task, picked_task);

  return picked_task;
}

/**
 * @brief 调度任务（NSJF）
 *
 * @see pick_next_task_nsjf
 * @see update_waiting_time_node
 * @see insert_node
 * @see print_sched_list
 * @see free_all_node
 * @see print_waiting_time
 *
 */
void schedule_task_nsjf()
{
  Task *picked_task;
  int count_task;
  float waiting_time_sum;
  float avg_waiting_time;
  int last_end_time = 0;
  int time = 0;
  
  count_task = count_node(head_task);
  
  while((picked_task = pick_next_task_nsjf(time)) != NULL)
  {
    int slice = picked_task->burst;
    time += slice;
    Task *new_sched_task = (Task *)malloc(sizeof(Task));
    *new_sched_task = *picked_task;
    //更新调度链表
    new_sched_task->start_time = last_end_time;
    new_sched_task->end_time = new_sched_task->start_time + slice;
    last_end_time = new_sched_task->end_time;
    insert_node(&head_sched, new_sched_task);

    //更新等待时间链表
    update_waiting_time_node(head_task, picked_task, slice);
  }

  printf("\n[NSJF]\n");
  print_sched_list(head_sched);
  print_waiting_time(head_waiting_time);

  struct Node *cur_node = head_waiting_time;
  while(cur_node != NULL)
  {
    waiting_time_sum += (cur_node->task)->waiting_time;
    cur_node = cur_node->next;
  }
  avg_waiting_time = waiting_time_sum / count_task;
  printf("Average Waiting Time: %.2f\n", avg_waiting_time);
  
  //释放所有节点
  free_all_node(head_sched);
  free_all_node(head_waiting_time);
}

// PSJF

/**
 * @brief 获取时间片 （PSJF）
 *
 * @param time
 * @param task
 * @return int
 */
int get_slice_psjf(int time, Task *task)
{
  int slice = time - task->start_time;

  return slice;
}

/**
 * @brief 是否从链表中删除任务（PSJF）
 *
 * @param slice
 * @param burst
 * @return true
 * @return false
 */
bool is_delete_node_psjf(int slice, int burst) { return (slice >= burst); }

/**
 * @brief 取下一个任务（PSJF）
 *
 * @see min
 * @see equals
 * @see add_tid
 * @see get_slice_psjf
 * @see is_delete_node_psjf
 * @see delete_node
 *
 * @param time
 * @return Task*
 */
Task *pick_next_task_psjf(int time, int *slice)
{
  Task *picked_task;
  Task *min_task;
  struct Node *cur_node;
  if(head_task == NULL)
  {
    return NULL;
  } else
  {
    picked_task = head_task->task;
  }

  cur_node = head_task;
  while((cur_node != NULL) && ((cur_node->task)->arrival_time <= time))
  {
    if(min((cur_node->task)->burst, picked_task->burst))
    {
      picked_task = cur_node->task;
    }
    cur_node = cur_node->next;
  }
  cur_node = head_task;
  min_task = head_task->task;
  while(cur_node != NULL)
  {
    if(min((cur_node->task)->burst, min_task->burst))
    {
      min_task = cur_node->task;
    }
    cur_node = cur_node->next;
  }

  //如果最小的是自己，则运行完，更新slice，更新自己的开始和结束时间，判断是否删除节点并删除，返回这个任务
  if((strcmp(picked_task->name, min_task->name)) == 0)
  {
    *slice += picked_task->burst;
    picked_task->start_time = time;
    picked_task->end_time = picked_task->start_time + *slice;
    if(is_delete_node_psjf(*slice, picked_task->burst))
    {
      delete_node(&head_task, picked_task);
      return picked_task;
    }
  } else
  {
    //如果最小的不是自己，就运行到最小的任务的到达时间，并更新自己的到达时间,当前的开始和结束时间,以及总运行时间
    *slice += min_task->arrival_time;
    picked_task->start_time = time;
    picked_task->end_time = picked_task->start_time + *slice;
    picked_task->arrival_time = picked_task->end_time;
    sub_slice(picked_task, *slice);

    return picked_task;
  }

}

/**
 * @brief 调度任务（PSJF）
 *
 * @see pick_next_task_psjf
 * @see update_waiting_time_node
 * @see insert_node
 * @see is_delete_node_psjf
 * @see sub_slice
 * @see print_sched_list
 * @see free_all_node
 * @see print_waiting_time
 *
 */
void schedule_task_psjf()
{
  Task *picked_task;
  int count_task;
  float waiting_time_sum;
  float avg_waiting_time;
  int time = 0;
  int slice = 0;

  count_task = count_node(head_task);

  //取任务，更新调度链表，更新等待链表，重置slice
  while((picked_task = pick_next_task_psjf(time, &slice)) != NULL)
  {
    Task *new_sched_task = (Task *)malloc(sizeof(Task));
    *new_sched_task = *picked_task;
    insert_node(&head_sched, new_sched_task);

    update_waiting_time_node(head_task, picked_task, slice);
    
    time += slice;
    slice = 0;
  }

  printf("\n[PSJF]\n");
  print_sched_list(head_sched);
  print_waiting_time(head_waiting_time);

  struct Node *cur_node = head_waiting_time;
  while(cur_node != NULL)
  {
    waiting_time_sum += (cur_node->task)->waiting_time;
    cur_node = cur_node->next;
  }
  avg_waiting_time = waiting_time_sum / count_task;
  printf("Average Waiting Time: %.2f\n", avg_waiting_time);
  
  //释放所有节点
  free_all_node(head_sched);
  free_all_node(head_waiting_time);
}

/**
 * @brief 从文件中按行读取数据，解析处理，增加任务到链表中
 *
 * @param filename 输入文件名
 */
void input_data(char *filename)
{
  FILE *fp_in;
  char read_line[READ_LINE_LEN];
  char *tok_ptr;
  char name[SIZE];
  int priority, burst;
  int arrival_time = 0;

  fp_in = fopen(filename, "r");
  while(fgets(read_line, READ_LINE_LEN, fp_in) != NULL)
  {
    tok_ptr = strtok(read_line, ",\n");
    strcpy(name, tok_ptr);

    tok_ptr = strtok(NULL, ",\n");
    priority = atoi(tok_ptr);

    tok_ptr = strtok(NULL, ",\n");
    burst = atoi(tok_ptr);

    add_task(name, priority, burst, arrival_time);

    ++arrival_time;
  }

  fclose(fp_in);
}

/**
 * @brief 打开输出文件
 *
 * 这里需要用到freopen，把标准控制台的输出重定向到文件。
 * 我们为了方便在输出到标准控制台和文件之间切换，这次采用这个方式。
 *
 *
 * @param filename 输出文件名
 */
void output_open(char *filename)
{
  file = freopen(filename, "w", stdout);
  if(file == NULL)
  {
    printf("reopen error.\n");
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief 关闭文件
 *
 */
void output_close()
{
  fclose(file);
}

/**
 * @brief 用法
 *
 * 如果命令行输入参数个数不符合要求，则打印用法，退出程序。
 *
 * @param argc 参数个数
 */
void usage(int argc)
{
  if (3 != argc)
  {
    printf("Usage: ./sched task.txt result.txt\n");
    exit(1);
  }
}

//主函数区域///////////////////////////////////////////////
/**
 * @brief 主函数入口
 *
 * @param argc 参数个数
 * @param argv 参数值数组
 * @return int 返回0为正常，其他为错误
 */
int main(int argc, char *argv[])
{
  //用法
  usage(argc);

  //注意：如果想输出到控制台，则需要注释掉这个函数
  //output_open(argv[2]);

  // FCFS
  init_head();
  input_data(argv[1]);
  schedule_task_fcfs();

  // RR
  init_head();
  input_data(argv[1]);
  schedule_task_rr();

  // NSJF
  init_head();
  input_data(argv[1]);
  schedule_task_nsjf();

  // PSJF
  init_head();
  input_data(argv[1]);
  schedule_task_psjf();

  //关闭文件
  //output_close();

  return 0;
}

