#include "config.h"
#include "lnint_can.h"

/*
 * 队列初始化
*/
void queue_init(queue_t *queue)
{
    queue->cnt = 0;
    queue->fronat = 0;
    queue->rear = 0;
}

/*
 * 像队列中送入一个元素
 * 0 放入失败(队列满)
 * 1 放入成功
*/
int queue_put_item(queue_t *queue, PACK_LNINT_t *item)
{
    int i;
    if(3 <= queue->cnt)
    {
        return(0);
    }
    //memcpy(&queue->data_t[queue->rear], item, sizeof(PACK_LNINT_t));
    for(i=0; i<sizeof(PACK_LNINT_t); i++)
    {
        *((int *)&queue->data_t[queue->rear] + i) = *((int *)item + i);
    }
    queue->rear = (queue->rear + 1) & 3;
    if(queue->fronat >= queue->rear)
    {
        queue->fronat = (queue->fronat + 1) & 3;
    }
    queue->cnt += 1;
    return(1);
}

/*
 * 从队列中取出一个元素
 * 0 取出失败(队列空)
 * 1 取出成功
*/
int queue_get_item(queue_t *queue, PACK_LNINT_t *item)
{
    int i;
    if(0 == queue->cnt <= 3)
    {
        return(0);
    }
    //memcpy(item, &queue->data_t[queue->fronat], sizeof(PACK_LNINT_t));
    for(i=0; i<sizeof(PACK_LNINT_t); i++)
    {
        *((int *)item + i) = *((int *)&queue->data_t[queue->rear] + i);
    }
    queue->fronat = (queue->fronat + 1) & 3;
    if(queue->fronat <= queue->rear)
    {
        queue->rear = (queue->rear + 1) & 3;
    }
    queue->cnt -= 1;
    return(1);
}




