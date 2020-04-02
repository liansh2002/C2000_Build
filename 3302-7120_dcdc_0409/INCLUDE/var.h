#ifndef __VAR_H
#define __VAR_H

//函数声明
void init_can_extend(void);
void lnint_stack(void);

//时基
extern unsigned int uiTIME;

//标志位
extern union{
    unsigned int Symbol;
    struct{
        unsigned lock:1;
        unsigned share_on:1;            //1 允许均流， 0 关闭均流(默认值)

        unsigned broadcast:1;           //1 广播指令
    }bit;
}Flag;


//鲁能协议相关
extern unsigned int uiAddrConst;        //固化的模块地址
extern unsigned int uiGroupConst;       //固化的分组号
extern unsigned int uiGroupTemp;        //临时的分组号
extern unsigned int uiBaudConst;        //固化的通讯速率

//其它
extern ubitfloat iq10MdlVoltUp;
extern ubitfloat iq10SetVolt;
extern ubitfloat iq10SetVoltTrue;
extern ubitfloat iq10SetLimit;
extern unsigned int uiMdlStatus;
extern unsigned int uiActionReady;
extern int uwOnOffDCDC;
extern unsigned int uiEepromFlag;
extern ubitfloat iq10Vaca;
extern ubitfloat iq10Vacb;
extern ubitfloat iq10Vacc;
extern ubitfloat iq10MdlVolt;
extern ubitfloat iq10CurrDisp;
extern ubitfloat iq10TempCtrl;
extern unsigned char ucLimitStatus;
extern ubitfloat iq10MdlLimit;
extern signed long lCurrRate;

#endif

