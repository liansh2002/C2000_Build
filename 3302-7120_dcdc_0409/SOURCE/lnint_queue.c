#include "config.h"
#include "lnint_can.h"

/*
 * ���г�ʼ��
*/
void queue_init(queue_t *queue)
{
    queue->cnt = 0;
    queue->fronat = 0;
    queue->rear = 0;
}

/*
 * �����������һ��Ԫ��
 * 0 ����ʧ��(������)
 * 1 ����ɹ�
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
 * �Ӷ�����ȡ��һ��Ԫ��
 * 0 ȡ��ʧ��(���п�)
 * 1 ȡ���ɹ�
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




