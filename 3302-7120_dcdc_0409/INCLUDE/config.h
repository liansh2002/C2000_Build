#ifndef __CONFIG_H
#define __CONFIG_H

//#define USE_LNINT                           //是否使用LNINT鲁能协议
#define dUSE_LNINT_POWER                         //鲁能恒功率
//#define dNOT_CAN                            //不使用CAN起机
//#define __SIGNAL_SAMPLE                   //单路AD采样(DCVOLT2, AN0 改为半差分采样)

//#define __DEBUG

#define dFREQ_TO_LOUT(f) ((3200000UL + (f / 2)) / f)    //频率直接转周期值
#define dQ10_DEAD_CURR_THR 3584l            //内管死区切换中，电流阈值
#define dQ10_DEAD_VOLT_THR 460800l          //内管死区切换中，电压阈值

//驱动频率相关
#define dPERIOD_80K 40000U
#define dPERIOD_70K 45714U
#define dPERIOD_MAX dPERIOD_70K

//PFC电压相关
#define dPFC_REF 400
#define dPFC_SET_MAX 420
#define dPFC_SET_MIN 320

#if defined(USE_LNINT)
    #define dSOFT_VER_DEC 100
    #define dSOFT_VER_HEX 0x0001
#else
    #define dSOFT_VER_DEC 000
    #define dSOFT_VER_HEX 0x0002
#endif

//规格相关
#define CURR_RATE 20480                     //额定电流20A
#define dQ10_VOUT_MIN (99l << 10)           //最小输出电压
#define dQ10_VOUT_MAX (750l << 10)          //最大输出电压
#define dQ10_VOUT_INIT (300l << 10)         //默认输出电压(起始电压)
#define dQ10_VOUT_FT_DEF (720l <<10)        //默认浮充电压
#define dQ10_OVP_DEF (760l << 10)           //OVP默认值
#define dQ10_OVP_MAX (760l << 10)           //OVP最大值
#define dQ10_OVP_MIN (300l << 10)           //OVP最小值

//DCDC常量
#define dPERIOD_OUT_MIN 0x23C0l             //周期最小值(频率大值0x23C0 = 350KHz)
#define dDEAD_ADJ1_MAX 410                  //用于控制内管和外管间占空比差值(数值越大内管占空比越小)
#define dDEAD_ADJ_MIN_NF 14                 //用于控制PWM2与PWM3之间的相位差(内管死区)，最小为14
#define dDEAD_ADJ_MIN_HF 30
#define dRESONANCE_POINT 0x5355             //谐振点

#if defined(dUSE_LNINT_POWER)
     //限流点
    #define dQ10_CURR_LIMIT_MAX 1920l           //最大限流点
    #define dQ10_CURR_LIMIT_MIN 100l            //最小限流点
    #define dQ10_POWER_LIMIT_MAX 1026l          //最大限功率点
    #define dQ10_POWER_LIMIT_MIN 238l           //最小限功率点
    #define dQ10_CURR_LIMIT_VREF (765l << 10)   //输出电压限流值

    //输出低压区间限流点
    #define dQ10_LOW_VOLT_TOP (409088l)         //低电压区间的上限 考虑采样误差，由330V降至329.5V
    #define dQ10_LOW_VOLT_BOTTOM (204288l)      //低电压区间的下限 考虑采样误差，由300V降至299.5V
    #define dQ10_CURR_LIMIT_VOLT_MIN 1715l      //低电压区间限流点
    #define dQ10_CURR_LIMIT_VOLT_STEP (((dQ10_CURR_LIMIT_MAX - dQ10_CURR_LIMIT_VOLT_MIN) << 20) / (dQ10_LOW_VOLT_TOP - dQ10_LOW_VOLT_BOTTOM))    //((330-300)/(2457-1920))*1024

    //升功率
    #define dQ10_POWER_GAIN_CURR_MIN 30720l
    #define dQ10_CURR_MAX ((dQ10_CURR_LIMIT_MAX * CURR_RATE) >> 10)
#else
    //限流点
    #define dQ10_CURR_LIMIT_MAX 2304l           //最大限流点
    #define dQ10_CURR_LIMIT_MIN 100l            //最小限流点
    #define dQ10_POWER_LIMIT_MAX 1026l          //最大限功率点
    #define dQ10_POWER_LIMIT_MIN 238l           //最小限功率点
    #define dQ10_CURR_LIMIT_VREF (765l << 10)   //输出电压限流值

    //输出低压区间限流点
    #define dQ10_LOW_VOLT_TOP (337408l)         //低电压区间的上限 考虑采样误差，由330V降至329.5V
    #define dQ10_LOW_VOLT_BOTTOM (306688l)      //低电压区间的下限 考虑采样误差，由300V降至299.5V
    #define dQ10_CURR_LIMIT_VOLT_MIN 1920l      //低电压区间限流点
    #define dQ10_CURR_LIMIT_VOLT_STEP (((dQ10_CURR_LIMIT_MAX - dQ10_CURR_LIMIT_VOLT_MIN) << 20) / (dQ10_LOW_VOLT_TOP - dQ10_LOW_VOLT_BOTTOM))    //((330-300)/(2457-1920))*1024

    //升功率
    #define dQ10_POWER_GAIN_CURR_MIN 35840l
    #define dQ10_CURR_MAX ((dQ10_CURR_LIMIT_MAX * CURR_RATE) >> 10)
#endif

//环温相关
#define dQ10_AMB_LIMIT_MAX (70l << 10)      //环温上限
#define dQ10_AMB_LIMIT_MIN (53l << 10)      //环温降额起始点

//功率相关
#define dQ10_POWER_GAIN (200l << 10)        //功率增加值

//其它
#define ELEMENTS_IN(array) __elements_in_##array                                            //创建一个名称
#define ELEMENTS_IN_DEF(array) const unsigned int __elements_in_##array = sizeof(array)     //定义一个变量，其初始值为传入参数的长度
#define ELEMENTS_IN_DECLARE(array) extern const unsigned int __elements_in_##array          //声明前一步定义的变量

//鲁能协议相关
#define dLNINT_ADDR_MIN 1
#define dLNINT_ADDR_MAX 127
#define dLNINT_GROUP_MIN 1
#define dLNINT_GROUP_MAX 126
#define dLNINT_BAUD_MIN 50
#define dLNINT_BAUD_MAX 500
#define dLNINT_BAUD_DEF 250

#define dEEPROM_ADDR_ADDR 176                   //本机地址
#define dEEPROM_GROUP_ADDR 180                  //分组号
#define dEEPROM_BAUD_ADDR 184                   //通讯速率
#define dEEPROM_SHARE_ADDR 188                  //均流开关

#define dEEPROM_ADDR_FLAG 0x1000
#define dEEPROM_GROUP_FLAG 0x2000
#define dEEPROM_BAUD_FLAG 0x4000
#define dEEPROM_SHARE_FLAG 0x8000

//状态代码定义(uiMdlStatus)
#define dDCDC_OV        0x0001          //输出过压
#define	dOVER_TEMP      0x0002          //过温
#define	dFAN_FAIL       0x0004          //风扇故障
#define dMDL_OFF        0x0008          //MDL关机
#define dPSM_OFF        0x0010          //通讯关机
//#define dDIO_FAIL       0x0020        //未使用
//#define dCOMM_FAIL      0x0040        //未使用
//#define dAC_FAIL        0x0080        //未使用
#define dSTATUS_PFC_FAIL 0x0100         //PFC故障
#define	dAVGCUR_FAIL    0x0200          //均流异常
#define dSTATUS_CAN_FAIL 0x0400         //CAN通讯异常    
#define dSHORT_CIRCUIT  0x0800          //输出短路
#define dEPROM_FAIL     0x1000          //EEPROM值错误(ID_ERROR)
#define	dSCI_FAIL       0x2000          //SCI通讯异常    
#define dSTATUS_GLED_FLASH 0x4000       //通讯绿灯闪烁
#define dDCDC_HVSD      0x8000          //输出过压锁死

//LED
#define dLED_YELLOW_IO GPIO19
#define dLED_RED_IO GPIO8
#define dLED_GREEN_IO GPIO9
#define dLED_YELLOW_ON() GpioDataRegs.GPASET.bit.dLED_YELLOW_IO = 1
#define dLED_YELLOW_OFF() GpioDataRegs.GPACLEAR.bit.dLED_YELLOW_IO = 1
#define dLED_RED_ON() GpioDataRegs.GPASET.bit.dLED_RED_IO = 1
#define dLED_RED_OFF() GpioDataRegs.GPACLEAR.bit.dLED_RED_IO = 1
#define dLED_GREEN_ON() GpioDataRegs.GPASET.bit.dLED_GREEN_IO = 1
#define dLED_GREEN_OFF() GpioDataRegs.GPACLEAR.bit.dLED_GREEN_IO = 1


#endif

