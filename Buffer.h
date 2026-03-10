#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

/*
============================================================
                通用静态环形队列（无 malloc）

特点：
1. 支持任意数据类型
2. 不使用动态内存
3. 由用户提供缓冲区
4. O(1) 时间复杂度

使用步骤：

1?? 定义缓冲区
    int buffer[10];

2?? 定义队列控制结构
    CircularQueue q;

3?? 初始化
    cq_init(&q, buffer, 10, sizeof(int));

4?? 入队 / 出队 / 查询

============================================================
*/

typedef struct
{
    void *buffer;          // 指向外部数据缓冲区
    size_t element_size;   // 单个元素大小（字节）
    size_t capacity;       // 最大可存储元素数量

    size_t head;           // 队头索引（指向当前可出队元素）
    size_t tail;           // 队尾索引（指向下一个可写入位置）
    size_t size;           // 当前已存储元素数量

} CircularQueue;


/*
------------------------------------------------------------
函数：cq_init

说明：
初始化队列（必须先调用）

参数：
queue         - 队列控制结构体指针
buffer        - 外部提供的数据存储数组
capacity      - 队列最大容量（元素个数）
element_size  - 单个元素大小

示例：
int buffer[10];
CircularQueue q;
cq_init(&q, buffer, 10, sizeof(int));
------------------------------------------------------------
*/
void cq_init(CircularQueue *queue,
             void *buffer,
             size_t capacity,
             size_t element_size);


/*
清空队列（不清除内存内容，仅重置索引）
*/
void cq_clear(CircularQueue *queue);


/*
------------------------------------------------------------
函数：cq_enqueue

说明：
向队列尾部插入一个元素

返回值：
0   成功
-1  队列已满

注意：
element 必须指向有效数据
------------------------------------------------------------
*/
int cq_enqueue(CircularQueue *queue, const void *element);


/*
------------------------------------------------------------
函数：cq_dequeue

说明：
从队列头部取出一个元素

参数：
out_element  - 用于接收数据的缓冲区

返回值：
0   成功
-1  队列为空
------------------------------------------------------------
*/
int cq_dequeue(CircularQueue *queue, void *out_element);


/*
------------------------------------------------------------
函数：cq_get

说明：
获取队列中指定位置元素（不出队）

参数：
index = 0 表示队头元素
index = size-1 表示队尾前一个元素

返回值：
0   成功
-1  越界
------------------------------------------------------------
*/
int cq_get(const CircularQueue *queue,
           size_t index,
           void *out_element);


/* 获取当前队列元素数量 */
size_t cq_size(const CircularQueue *queue);

/* 判断队列是否为空 */
int cq_is_empty(const CircularQueue *queue);

/* 判断队列是否已满 */
int cq_is_full(const CircularQueue *queue);

#endif