#ifndef __LNINT_CAN_H
#define __LNINT_CAN_H

#define dSET_SUCCESS 0xFF00
#define dSET_FAILURE 0xFF01
#define dLNINIT_VER 0x0901      //LNINT规约版本号V1.09

typedef struct{
    unsigned SA:8;              //7:0   源域
    unsigned GS:8;              //15:8  目标域
    unsigned CMD:8;             //23:16 指令域
    unsigned PRIORITY:5;        //28:24 优先权域
    unsigned AAM:1;             //29    1自动应答模式，0正常发送模式
    unsigned AME:1;             //30    1接收屏蔽使能，0接收屏蔽禁止
    unsigned IDE:1;             //31    1扩展帧，0标准帧
}MSGID_LNINT_BIT_t;
typedef union{
    void *p;
    MSGID_LNINT_BIT_t *bit;
}MSGID_LNINT_t;

typedef struct{
    unsigned int cmd;           //指令
    unsigned int target;        //目标
    unsigned int source;        //来源
    unsigned int u16_data[4];   //数据
}PACK_LNINT_t;

//2B编译器，下面的宏不使用变量(INDEX)的话，会报等式恒成立
#define dLNINT_BYTE(n) (__INDEX[n] & 1) ? (dLNINT_DATA[(n) >> 1] & 0x00FF) : (dLNINT_DATA[(n) >> 1] >> 8)
//#define dLNINT_BYTE(n) (n & 1) ? (dLNINT_DATA[(n) >> 1] & 0x00FF) : (dLNINT_DATA[(n) >> 1] >> 8)
#define dLNINT_WORD(n) ((dLNINT_DATA[n] >> 8) | (dLNINT_DATA[n] << 8))  //需要交换高低字节
#define dQ10_TO_LNINT(data) ((((10l * data) + 512) >> 18) | ((((10l * data) + 512) >> 2) & 0xFF00))

typedef struct{
    PACK_LNINT_t data_t[4];
    unsigned int fronat;
    unsigned int rear;
    unsigned int cnt;
}queue_t;

//函数声明
void queue_init(queue_t *queue);
int queue_put_item(queue_t *queue, PACK_LNINT_t *item);
int queue_get_item(queue_t *queue, PACK_LNINT_t *item);


#endif

