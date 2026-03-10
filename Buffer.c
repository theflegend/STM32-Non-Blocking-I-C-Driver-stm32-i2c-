#include <string.h>
#include "Buffer.h"

/*
初始化队列控制结构
*/
void cq_init(CircularQueue *queue,
             void *buffer,
             size_t capacity,
             size_t element_size)
{
    queue->buffer = buffer;
    queue->capacity = capacity;
    queue->element_size = element_size;

    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}


/*
重置队列状态
*/
void cq_clear(CircularQueue *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}


/*
入队操作

步骤：
1. 判断是否已满
2. 计算写入地址
3. 拷贝数据
4. 更新 tail
5. 更新 size
*/
int cq_enqueue(CircularQueue *queue, const void *element)
{
    if (cq_is_full(queue))
        return -1;

    void *target = (char*)queue->buffer +
                   (queue->tail * queue->element_size);

    memcpy(target, element, queue->element_size);

    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size++;

    return 0;
}


/*
出队操作

步骤：
1. 判断是否为空
2. 计算读取地址
3. 拷贝数据
4. 更新 head
5. 更新 size
*/
int cq_dequeue(CircularQueue *queue, void *out_element)
{
    if (cq_is_empty(queue))
        return -1;

    void *source = (char*)queue->buffer +
                   (queue->head * queue->element_size);

    memcpy(out_element, source, queue->element_size);

    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;

    return 0;
}


/*
获取指定位置元素（不改变队列状态）
*/
int cq_get(const CircularQueue *queue,
           size_t index,
           void *out_element)
{
    if (index >= queue->size)
        return -1;

    size_t real_index =
        (queue->head + index) % queue->capacity;

    void *source = (char*)queue->buffer +
                   (real_index * queue->element_size);

    memcpy(out_element, source, queue->element_size);

    return 0;
}


/*
返回当前元素数量
*/
size_t cq_size(const CircularQueue *queue)
{
    return queue->size;
}


/*
判断是否为空
*/
int cq_is_empty(const CircularQueue *queue)
{
    return (queue->size == 0);
}


/*
判断是否已满
*/
int cq_is_full(const CircularQueue *queue)
{
    return (queue->size == queue->capacity);
}