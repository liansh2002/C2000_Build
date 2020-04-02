#ifdef USE_LNINT

#include "IQmathLib.h"
#include "DSP280x_Device.h"
#include "config.h"
#include "lnint_can.h"
#include "var.h"

//自用变量
static const unsigned int __INDEX[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static MSGID_LNINT_t t_msgid_receive_reg, t_msgid_transmit_reg;         //收发数据邮箱ID寄存器
static unsigned int ui_30s_count = 0;                                   //30秒周期计数(B2-2)
static unsigned int ui_3s_count = 0;                                    //3秒周期计数(B2-1及B3)
static union{
    unsigned int symbol;
    struct{
        unsigned vin_ov:1;      //1 输入过压
        unsigned vin_uv:1;      //1 输入欠压
        unsigned vout_ov:1;     //1 输出过压
        unsigned vout_uv:1;     //1 输出欠压
        
        unsigned oc:1;          //1 输出过流
        unsigned ot:1;          //1 过温
        unsigned fan:1;         //1 风扇
        unsigned other:1;       //1 其它告警
        
        unsigned empty:8;       //高8位不使用
    }bit;
}warn_lnint = {0};
static union{
    unsigned int symbol;
    struct{
        unsigned share:1;       //1 均流开启
        unsigned derate:1;      //1 降额开启
        
        unsigned empty:14;      //  未使用
    }bit;
}status_lnint = {0};
queue_t lnint_answer_q, lnint_auto_q;   //发送数据队列

/******************************************
baud = SYSCLKOUT / (BRPREG + 1) * ((TSEG1REG + 1) + (TSEG2REG + 1) + 1)
Tsjw同步跳转段宽度：Tsjw = Tscl * (2 * SJW[1] + 1 * SJW[0]).最大为4，必须小于TSEG1以及TSEG2。
******************************************/
void reset_can_baud(unsigned int ui_can_baud)
{
    struct ECAN_REGS ECanaShadow;                   //依据数据手册，为防止CAN模块出现不可预测行为，对CAN模块读写不应使用位操作

    ECanaShadow.CANBTC.all = 0;
    ECanaShadow.CANMD = ECanaRegs.CANMD;
    ECanaShadow.CANME = ECanaRegs.CANME;            //缓存使能的邮箱
    switch(ui_can_baud)
    {
        case 50:    //50KHz波特率
        {
            ECanaShadow.CANBTC.bit.BRPREG = 124;    //Tq = 1.25uS    
            ECanaShadow.CANBTC.bit.TSEG1REG = 10;
            ECanaShadow.CANBTC.bit.TSEG2REG = 3;
            ECanaShadow.CANBTC.bit.SAM = 1;         //0:单次采样，1:三次采样(只要分频BRP>4就可以使用)
            ECanaShadow.CANBTC.bit.SJWREG = 2;
        }break;

        case 100:   //100KHz波特率
        {
            ECanaShadow.CANBTC.bit.BRPREG = 74;     //Tq = 0.75uS    
            ECanaShadow.CANBTC.bit.TSEG1REG = 10;
            ECanaShadow.CANBTC.bit.TSEG2REG = 3;
            ECanaShadow.CANBTC.bit.SAM = 1;         //0:单次采样，1:三次采样(只要分频BRP>4就可以使用)
            ECanaShadow.CANBTC.bit.SJWREG = 2;
        }break;

        case 125:   //125KHz波特率
        {
            ECanaShadow.CANBTC.bit.BRPREG = 49;     //Tq = 0.5uS    
            ECanaShadow.CANBTC.bit.TSEG1REG = 10;
            ECanaShadow.CANBTC.bit.TSEG2REG = 3;
            ECanaShadow.CANBTC.bit.SAM = 1;         //0:单次采样，1:三次采样(只要分频BRP>4就可以使用)
            ECanaShadow.CANBTC.bit.SJWREG = 2;
        }break;

        case 250:   //250KHz波特率
        {
            ECanaShadow.CANBTC.bit.BRPREG = 24;     //Tq = 0.25uS    
            ECanaShadow.CANBTC.bit.TSEG1REG = 10;
            ECanaShadow.CANBTC.bit.TSEG2REG = 3;
            ECanaShadow.CANBTC.bit.SAM = 1;         //0:单次采样，1:三次采样(只要分频BRP>4就可以使用)
            ECanaShadow.CANBTC.bit.SJWREG = 2;
        }break;

        case 500:   //500KHz波特率
        {
            ECanaShadow.CANBTC.bit.BRPREG = 9;      //Tq = 0.1uS    
            ECanaShadow.CANBTC.bit.TSEG1REG = 13;
            ECanaShadow.CANBTC.bit.TSEG2REG = 4;
            ECanaShadow.CANBTC.bit.SAM = 1;         //0:单次采样，1:三次采样(只要分频BRP>4就可以使用)
            ECanaShadow.CANBTC.bit.SJWREG = 2;
        }break;

        default:
        {
        }return;    //不做任何处理
    }
    ECanaRegs.CANMD.all = 0;
    ECanaRegs.CANME.all = 0;                    //先关闭所有邮箱，待切换完成打开
    ECanaRegs.CANRMP.all = 0xFFFFFFFF;          //清除所有接受完成标志
    
    EALLOW;
    ECanaRegs.CANMC.bit.CCR = 1;
    while(!ECanaRegs.CANES.bit.CCE);            //当CCE为1时，CANBTC解除锁定成功(由于CCE需要检测到RX脚高电平磁能置1，所以需要等待)

    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

    ECanaRegs.CANMC.bit.CCR = 0;                //更改CAN设置
    while(ECanaRegs.CANES.bit.CCE);
    EDIS;

    ECanaRegs.CANME.all = ECanaShadow.CANME.all;    //恢复所有邮箱
    ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;    //恢复邮箱收/发选择
    
}

/**************************************
CAN初始化补充，根据LNINT协议创建
**************************************/
void init_can_extend(void)
{
    t_msgid_receive_reg.p = (void *)&ECanaMboxes.MBOX2.MSGID;
    t_msgid_transmit_reg.p = (void *)&ECanaMboxes.MBOX6.MSGID;
    
    //增加LNINT协议邮箱
    t_msgid_receive_reg.bit->PRIORITY = 0x18;
    t_msgid_receive_reg.bit->IDE = 1;           //扩展帧
    t_msgid_receive_reg.bit->AME = 1;           //接收屏蔽使能
    t_msgid_receive_reg.bit->AAM = 0;           //进制自动应答
    
    t_msgid_transmit_reg.bit->PRIORITY = 0x18;
    t_msgid_transmit_reg.bit->IDE = 1;          //扩展帧
    t_msgid_transmit_reg.bit->AME = 1;          //接收屏蔽使能
    t_msgid_transmit_reg.bit->AAM = 0;          //进制自动应答

    ECanaRegs.CANME.all = 0x00000033 | 0x00000044;  //打开2，6
    ECanaRegs.CANMD.all = 0x00000003 | 0x00000004;  //mailbox2用于接受LNINT协议
    
    //CAN速率调整
    reset_can_baud(uiBaudConst);                    //重设CAN速率

    //队列初始化
    queue_init(&lnint_answer_q);
    queue_init(&lnint_auto_q);
}
/**************************************
lnint模拟量上报(B2-1)
**************************************/
void lnint_analog_b21(PACK_LNINT_t *data_t)
{
    unsigned int ui_tmp;
    
    data_t->u16_data[0] = 0x0100;
    
    //A相电压
    ui_tmp = ((iq10Vaca.lData * 10) + 512) >> 10;
    data_t->u16_data[0] |= (ui_tmp & 0x00FF);
    data_t->u16_data[1] = (ui_tmp << 8);
    
    //B相电压
    ui_tmp = ((iq10Vacb.lData * 10) + 512) >> 10;
    data_t->u16_data[1] |= (ui_tmp & 0x00FF);
    data_t->u16_data[2] = (ui_tmp << 8);
    
    //C相电压
    ui_tmp = ((iq10Vacc.lData * 10) + 512) >> 10;
    data_t->u16_data[2] |= (ui_tmp & 0x00FF);
    data_t->u16_data[3] = (ui_tmp << 8);

    data_t->u16_data[3] |= 0xFF00;
}
/**************************************
lnint模拟量上报(B2-2)
**************************************/
void lnint_analog_b22(PACK_LNINT_t *data_t)
{
    unsigned int ui_tmp;

    data_t->u16_data[0] = 0x0200;

    //输出电压
    ui_tmp = ((iq10MdlVolt.lData * 10) + 512) >> 10;
    data_t->u16_data[0] |= (ui_tmp & 0x00FF);
    data_t->u16_data[1] = (ui_tmp << 8);
    
    //输出电流
    ui_tmp = ((iq10CurrDisp.lData * 10) + 512) >> 10;
    data_t->u16_data[1] |= (ui_tmp & 0x00FF);
    data_t->u16_data[2] = (ui_tmp << 8);
    
    //环境温度
    ui_tmp = ((iq10TempCtrl.lData * 10) + 512512) >> 10;        //客户环温有50度偏置，再加512用于四舍五入
    data_t->u16_data[2] |= (ui_tmp & 0x00FF);
    data_t->u16_data[3] = (ui_tmp << 8);

    data_t->u16_data[3] |= 0xFF00;
}

/**************************************
lnint告警量上报(B3)
**************************************/
unsigned int lnint_warn_b3(PACK_LNINT_t *data_t)
{
    static unsigned int warn_back = 0;
    static unsigned int status_back = 0;
    
    //告警检查
        //输入过压
        
        //输入欠压
        
        //输出过压
    if(uiMdlStatus & (dDCDC_OV | dDCDC_HVSD))
    {
        warn_lnint.bit.vout_ov = 1;
    }
    else
    {
        warn_lnint.bit.vout_ov = 0;
    }
        //输出欠压
    
        //输出短路 
    if(uiMdlStatus & dSHORT_CIRCUIT)
    {
        warn_lnint.bit.oc = 1;
    }
    else
    {
        warn_lnint.bit.oc = 1;
    }
        //过温
    if(uiMdlStatus & dOVER_TEMP)
    {
        warn_lnint.bit.ot = 1;
    }
    else
    {
        warn_lnint.bit.ot = 0;
    }
        //风扇
    if(uiMdlStatus & dFAN_FAIL)
    {
        warn_lnint.bit.fan = 1;
    }
    else
    {
        warn_lnint.bit.fan = 0;
    }
    
    //状态监测
    if(Flag.bit.share_on)                       //允许均流
    {
        status_lnint.bit.share = 1;
    }
    else                                        //禁止均流
    {
        status_lnint.bit.share = 0;
    }
    if(ucLimitStatus & 0x06)                    //温度限流或输入限流
    {
        status_lnint.bit.derate = 1;
    }
    else
    {
        status_lnint.bit.derate = 0;
    }

    //写数据
    data_t->u16_data[0] = uiGroupConst;
    if(0 == (uiMdlStatus & (dPSM_OFF | dMDL_OFF)))  //当前开机
    {
        data_t->u16_data[0] |= 0x0100;
    } 
    data_t->u16_data[1] = status_lnint.symbol;  //状态位
    data_t->u16_data[2] = 0xFFFF;               //未使用
    data_t->u16_data[3] = warn_lnint.symbol;    //告警位

    //状态检查
    if((warn_back != warn_lnint.symbol) || (status_back != status_lnint.symbol))        //有发生状态更新
    {
        //更新备份数据
        warn_back = warn_lnint.symbol;
        status_back = status_lnint.symbol;
        return(1);
    }
    return(0);
}


/**************************************
lnint协议处理
**************************************/
#define dLNINT_DATA lnint_pack.u16_data             //在使用宏dGET_BYTE及dGET_WORD前，需先定义数据来源    
void lnint_process(PACK_LNINT_t lnint_pack)
{
    ubitfloat iq10SetVoltTmp, iq10SetLimitTmp;
    unsigned int ui_control;
    long l_tmp;
    PACK_LNINT_t answer_t;                          //应答数据缓存

    Flag.bit.broadcast = 0;                         //默认按地址设置指令
    if(lnint_pack.target & 0x0080)                  //按组设置
    {
        if((lnint_pack.target != (uiGroupTemp | 0x0080)) && (0x00FF != lnint_pack.target))
        {
            return;
        }
        Flag.bit.broadcast = 1;                     //广播(按组设置指令)
    }
    if(lnint_pack.target != uiAddrConst)            //按地址设置
    {
        return;
    }
    //应答数据前置处理
    answer_t.cmd = lnint_pack.cmd | 0x0010;         //模块应答为0xBn
    answer_t.source = uiAddrConst;
    answer_t.target = lnint_pack.source;
    answer_t.u16_data[0] = 0xFFFF;
    answer_t.u16_data[1] = 0xFFFF;
    answer_t.u16_data[2] = 0xFFFF;
    answer_t.u16_data[3] = 0xFFFF;
    
    //协议处理
    switch(lnint_pack.cmd)
    {
        case 0xA1:                                      //直接固化地址
        {
            if((dLNINT_ADDR_MAX >= dLNINT_BYTE(0)) && (dLNINT_ADDR_MIN <= dLNINT_BYTE(0)))
            {
                if(dLNINT_BYTE(0) != uiAddrConst)
                {
                    uiAddrConst = dLNINT_BYTE(0);
                    uiEepromFlag |= dEEPROM_ADDR_FLAG;  //有变化则写入EEPROM
                }
                answer_t.u16_data[0] = dSET_SUCCESS;    //设置成功
            }
            else
            {
                answer_t.u16_data[0] = dSET_FAILURE;    //设置失败
            }
            answer_t.source = uiAddrConst;              //更新回传的本机地址
            if(!Flag.bit.broadcast)
            {
                queue_put_item(&lnint_answer_q, &answer_t); //送入发送队列
            }    
        }break;

        case 0xA2:                                      //固化分组号(无应答)
        {
            if((dLNINT_GROUP_MAX >= dLNINT_BYTE(0)) && (dLNINT_GROUP_MIN <= dLNINT_BYTE(0)))
            {
                if(dLNINT_BYTE(0) != uiGroupConst)
                {
                    uiGroupConst = dLNINT_BYTE(0);
                    uiEepromFlag |= dEEPROM_GROUP_FLAG;
                }
                uiGroupTemp = uiGroupConst;             //临时组号同步变更
            }
        }break;

        case 0xA3:
        case 0xA4:                                      //单机(分组控制)(无应答)
        {
            ui_control = 0;                             //默认为非针对本机的控制
            
            iq10SetVoltTmp.lData = dLNINT_WORD(0);      //提取电压设置值
            iq10SetVoltTmp.lData = ((iq10SetVoltTmp.lData << 10) + 5) / 10;     //转IQ10格式
            
            iq10SetLimitTmp.lData = dLNINT_WORD(1);     //提取电流设置值
            iq10SetLimitTmp.lData = ((iq10SetLimitTmp.lData << 10) + 5) / 10;   //转IQ10格式百分比值
            iq10SetLimitTmp.lData = _IQ10div(iq10SetLimitTmp.lData, CURR_RATE);
            
            l_tmp = dLNINT_DATA[2];
            l_tmp <<= 16;
            l_tmp |= dLNINT_DATA[3];                    //提取组标识数据

            if(0xFF != lnint_pack.target)               //非群控，针对本机
            {
                ui_control = 1;
            }
            else if(l_tmp & (1Ul << ((uiGroupTemp - 1) & 31)))   //群控，且针对本组
            {
                ui_control = 1;
            }
            
            if(1 == ui_control)                         //需要控制
            {
                //电压范围检查
                if(iq10SetVoltTmp.lData > (iq10MdlVoltUp.lData - 5120))
                {
                    iq10SetVoltTmp.lData = (iq10MdlVoltUp.lData - 5120);
                    if(iq10SetVoltTmp.lData > dQ10_VOUT_MAX)
                    {
                        iq10SetVoltTmp.lData = dQ10_VOUT_MAX;
                    }
                }
                else if(iq10SetVoltTmp.lData < dQ10_VOUT_MIN)
                {
                    iq10SetVoltTmp.lData = dQ10_VOUT_MIN;
                }
                //电流范围检查
                if(iq10SetLimitTmp.lData > dQ10_CURR_LIMIT_MAX)
                {
                    iq10SetLimitTmp.lData = dQ10_CURR_LIMIT_MAX;
                }
                else if(iq10SetLimitTmp.lData < dQ10_CURR_LIMIT_MIN)
                {
                    iq10SetLimitTmp.lData = dQ10_CURR_LIMIT_MIN;
                }
                
                //更新实际值
                //由于此指令没有回传内容，按协议要求设置值超出上/下限时，按限值执行
                iq10SetVolt.lData = iq10SetVoltTmp.lData;
                iq10SetLimit.lData = iq10SetLimitTmp.lData;
                
                if(0xA3 == lnint_pack.cmd)          //开机
                {
                    uiMdlStatus &= ~(dMDL_OFF | dPSM_OFF);
                    uwOnOffDCDC = 0x0;
                }
                else                                //关机
                {
                    uiMdlStatus |= (dMDL_OFF | dPSM_OFF);
                    uiActionReady = 0;
                    uwOnOffDCDC = 0x01;
                    iq10SetVoltTrue.lData = dQ10_VOUT_INIT;
                }
            }
        }break;

        case 0xA5:                                  //设置通讯速率
        {
            if((dLNINT_BAUD_MAX >= dLNINT_WORD(0)) && (dLNINT_BAUD_MIN <= dLNINT_WORD(0)))
            {
                if(dLNINT_WORD(0) != uiBaudConst)
                {
                    uiBaudConst = dLNINT_WORD(0);   //写入，但需要断电重启后才生效
                    uiEepromFlag |= dEEPROM_BAUD_FLAG;
                }
                answer_t.u16_data[0] = dSET_SUCCESS;
            }
            else
            {
                answer_t.u16_data[0] = dSET_FAILURE;
            }
            queue_put_item(&lnint_answer_q, &answer_t);
        }break;

        case 0xA6:                                  //临时变更模块组号(无应答)
        {
            if(0 != dLNINT_BYTE(1))                 //恢复临时分组号为固化值
            {
                uiGroupTemp = uiGroupConst;
            }
            else if((dLNINT_ADDR_MAX >= dLNINT_BYTE(0)) && (dLNINT_ADDR_MIN <= dLNINT_BYTE(0)))
            {
                uiGroupTemp = dLNINT_BYTE(0);
            }
        }break;

        case 0xA7:                                  //查询模块上送信息
        {
            if(0x01 == dLNINT_BYTE(0))              //回传计时归零(无应答)
            {
                ui_30s_count = 0;
                ui_3s_count = 0;
            }
            else if(0x01 == dLNINT_BYTE(1))         //B2-01帧
            {
                lnint_analog_b21(&answer_t);
                if(Flag.bit.broadcast)
                {
                    queue_put_item(&lnint_answer_q, &answer_t);
                }    
            }
            else if(0x01 == dLNINT_BYTE(2))         //B2-02帧
            {
                lnint_analog_b22(&answer_t);
                if(Flag.bit.broadcast)
                {
                    queue_put_item(&lnint_answer_q, &answer_t);
                }    
            }
            else if(0x01 == dLNINT_BYTE(3))         //B3帧
            {
                lnint_warn_b3(&answer_t);
                if(Flag.bit.broadcast)
                {
                    queue_put_item(&lnint_answer_q, &answer_t);
                }    
            }
            
        }break;

        case 0xA8:                                  //查询规约版本号及极限值
        {
            answer_t.cmd = 0x00B8;                  //B8数据
            answer_t.u16_data[0] = dLNINIT_VER;                                             //规约版本
            answer_t.u16_data[1] = dQ10_TO_LNINT(_IQ10mpy(dQ10_CURR_LIMIT_MIN, CURR_RATE)); //最小限流电流值
            answer_t.u16_data[2] = dQ10_TO_LNINT(_IQ10mpy(dQ10_CURR_LIMIT_MAX, CURR_RATE)); //最大限流电流值
            answer_t.u16_data[3] = dQ10_TO_LNINT(_IQ10mpy(iq10MdlLimit.lData, lCurrRate));  //降额后最大限流值
            if(!Flag.bit.broadcast)
            {
                queue_put_item(&lnint_answer_q, &answer_t);
            }    

            answer_t.cmd = 0x00B9;                  //B9数据
            answer_t.u16_data[0] = dQ10_TO_LNINT(dQ10_VOUT_MIN);                            //最小输出电压
            answer_t.u16_data[1] = dQ10_TO_LNINT(dQ10_VOUT_MAX);                            //最大输出电压
            answer_t.u16_data[2] = dQ10_TO_LNINT(dQ10_AMB_LIMIT_MIN);                       //降额点(环温)
            answer_t.u16_data[3] = dSOFT_VER_HEX;                                           //软件版本
            if(!Flag.bit.broadcast)
            {
                queue_put_item(&lnint_answer_q, &answer_t);
            }    
        }break;

        case 0xAF:                                  //均流
        {
            if(0 == dLNINT_BYTE(0))                 //禁止均流
            {
                if(Flag.bit.share_on)
                {
                    Flag.bit.share_on = 0;
                    uiEepromFlag |= dEEPROM_SHARE_FLAG;
                    answer_t.u16_data[0] = 0xFF00;  //回传禁止均流
                }
            }
            else                                    //允许均流(此指令回传只有开/关两种状态，没有设置失败的选项)
            {
                if(!Flag.bit.share_on)
                {
                    Flag.bit.share_on = 1;
                    uiEepromFlag |= dEEPROM_SHARE_FLAG;
                    answer_t.u16_data[0] = 0xFF01;  //回传允许均流
                }
            }
            if(!Flag.bit.broadcast)
            {
                queue_put_item(&lnint_answer_q, &answer_t);
            }    
        }break;

        default:
        {
        }break;
    }
}
void lnint_time(void)
{
    static unsigned int ui_stime = 0;
    PACK_LNINT_t auto_t;

    if((unsigned int)(uiTIME - ui_stime) < 833)         //<10mS
    {
        return;
    }
    ui_stime = uiTIME;

    //前置处理
    auto_t.source = uiAddrConst;
    auto_t.target = uiGroupTemp;                        //主动上报数据目标地址为本机分组号

    //自动发送数据
    ui_30s_count++;
    ui_3s_count++;
    if(lnint_warn_b3(&auto_t))                          //有变化则回送
    {
        auto_t.cmd = 0xB3;
        queue_put_item(&lnint_auto_q, &auto_t);         //送发送队列
    }
    else if(ui_30s_count >= 3000)                       //B2-02
    {
        ui_30s_count = 0;
        auto_t.cmd = 0xB2;
        lnint_analog_b22(&auto_t);
        queue_put_item(&lnint_auto_q, &auto_t);         //送发送队列
    }
    else if(ui_3s_count >= 300)                         //B2-01,B3
    {
        ui_3s_count = 0;
        auto_t.cmd = 0xB2;
        lnint_analog_b21(&auto_t);
        queue_put_item(&lnint_auto_q, &auto_t);         //送发送队列
        
        lnint_warn_b3(&auto_t);                         //B3更新3秒定时上送
        queue_put_item(&lnint_auto_q, &auto_t);         //送发送队列
    }
    else
    {
        return;
    }
}

/**************************************
lnint协议栈
**************************************/
void lnint_stack(void)
{
    struct ECAN_REGS ECanaShadow;
    PACK_LNINT_t lnint_data_t;
    
    ECanaShadow.CANRMP.all = ECanaRegs.CANRMP.all;
    ECanaShadow.CANTA.all = ECanaRegs.CANTA.all;        //缓存邮箱状态
    
    if(ECanaShadow.CANRMP.bit.RMP2)                     //邮箱2有数据
    {
        lnint_data_t.cmd = t_msgid_receive_reg.bit->CMD & 0x00FF;
        lnint_data_t.source = t_msgid_receive_reg.bit->GS & 0x00FF;
        lnint_data_t.target = t_msgid_receive_reg.bit->SA & 0x00FF;
        lnint_data_t.u16_data[0] = ECanaMboxes.MBOX2.MDL.word.HI_WORD;      //bit[31:16]--byte0,byte1
        lnint_data_t.u16_data[1] = ECanaMboxes.MBOX2.MDL.word.LOW_WORD;     //bit[15:00]--byte2,byte3
        lnint_data_t.u16_data[2] = ECanaMboxes.MBOX2.MDH.word.HI_WORD;      //bit[31:16]--byte4,byte5
        lnint_data_t.u16_data[3] = ECanaMboxes.MBOX2.MDH.word.LOW_WORD;     //bit[15:00]--byte6,byte7
        
        ECanaShadow.CANRMP.all = (1Ul << 2);
        ECanaRegs.CANRMP.all = ECanaShadow.CANRMP.all;  //清除接收完成标志

        lnint_process(lnint_data_t);                    //协议处理
    }

    //协议中定时处理
    lnint_time();

    //检查发送队列并发送
    //必须先检查应答队列，再检查自动发送队列
    if(!ECanaRegs.CANTA.bit.TA6 || ECanaRegs.CANTRS.bit.TRS6)
    {
        return;
    }
    if(0 == queue_get_item(&lnint_answer_q, &lnint_data_t) &&\
        0 == queue_get_item(&lnint_auto_q, &lnint_data_t))              //先检查应答队列
    {
        return;
    }
    ECanaRegs.CANME.all = (ECanaRegs.CANME.all & 0xFFFFFFBF);           //先关闭邮箱6
    t_msgid_transmit_reg.bit->CMD = lnint_data_t.cmd;
    t_msgid_transmit_reg.bit->GS = lnint_data_t.target;
    t_msgid_transmit_reg.bit->SA = lnint_data_t.source;
    ECanaRegs.CANME.all = (ECanaRegs.CANME.all | 0x00000040);           //打开邮箱6
    
    ECanaMboxes.MBOX2.MDL.word.HI_WORD = lnint_data_t.u16_data[0];      //bit[31:16]--byte0,byte1
    ECanaMboxes.MBOX2.MDL.word.LOW_WORD = lnint_data_t.u16_data[1];     //bit[15:00]--byte2,byte3
    ECanaMboxes.MBOX2.MDH.word.HI_WORD = lnint_data_t.u16_data[2];      //bit[31:16]--byte4,byte5
    ECanaMboxes.MBOX2.MDH.word.LOW_WORD = lnint_data_t.u16_data[3];     //bit[15:00]--byte6,byte7
    
    ECanaRegs.CANTRS.all = (ECanaRegs.CANTRS.all | 0x00000040);         //启动发送
}


#else
void init_can_extend(void)
{
}
void lnint_stack(void)
{
}
#endif





