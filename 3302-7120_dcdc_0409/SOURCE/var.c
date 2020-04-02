#include "config.h"

//时基
unsigned int uiTIME = 0;

union{
    unsigned int Symbol;
    struct{
        unsigned lock:1;
        unsigned share_on:1;            //1 允许均流， 0 关闭均流(默认值)

        unsigned broadcast:1;           //1 广播指令
    }bit;
}Flag = {0};

//鲁能协议相关
unsigned int uiAddrConst;               //固化的模块地址
unsigned int uiGroupConst;              //固化的分组号
unsigned int uiGroupTemp;               //临时的分组号
unsigned int uiBaudConst;               //固化的通讯速率


