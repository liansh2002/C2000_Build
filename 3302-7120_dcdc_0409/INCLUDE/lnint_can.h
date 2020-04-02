#ifndef __LNINT_CAN_H
#define __LNINT_CAN_H

#define dSET_SUCCESS 0xFF00
#define dSET_FAILURE 0xFF01
#define dLNINIT_VER 0x0901      //LNINT��Լ�汾��V1.09

typedef struct{
    unsigned SA:8;              //7:0   Դ��
    unsigned GS:8;              //15:8  Ŀ����
    unsigned CMD:8;             //23:16 ָ����
    unsigned PRIORITY:5;        //28:24 ����Ȩ��
    unsigned AAM:1;             //29    1�Զ�Ӧ��ģʽ��0��������ģʽ
    unsigned AME:1;             //30    1��������ʹ�ܣ�0�������ν�ֹ
    unsigned IDE:1;             //31    1��չ֡��0��׼֡
}MSGID_LNINT_BIT_t;
typedef union{
    void *p;
    MSGID_LNINT_BIT_t *bit;
}MSGID_LNINT_t;

typedef struct{
    unsigned int cmd;           //ָ��
    unsigned int target;        //Ŀ��
    unsigned int source;        //��Դ
    unsigned int u16_data[4];   //����
}PACK_LNINT_t;

//2B������������ĺ겻ʹ�ñ���(INDEX)�Ļ����ᱨ��ʽ�����
#define dLNINT_BYTE(n) (__INDEX[n] & 1) ? (dLNINT_DATA[(n) >> 1] & 0x00FF) : (dLNINT_DATA[(n) >> 1] >> 8)
//#define dLNINT_BYTE(n) (n & 1) ? (dLNINT_DATA[(n) >> 1] & 0x00FF) : (dLNINT_DATA[(n) >> 1] >> 8)
#define dLNINT_WORD(n) ((dLNINT_DATA[n] >> 8) | (dLNINT_DATA[n] << 8))  //��Ҫ�����ߵ��ֽ�
#define dQ10_TO_LNINT(data) ((((10l * data) + 512) >> 18) | ((((10l * data) + 512) >> 2) & 0xFF00))

typedef struct{
    PACK_LNINT_t data_t[4];
    unsigned int fronat;
    unsigned int rear;
    unsigned int cnt;
}queue_t;

//��������
void queue_init(queue_t *queue);
int queue_put_item(queue_t *queue, PACK_LNINT_t *item);
int queue_get_item(queue_t *queue, PACK_LNINT_t *item);


#endif

