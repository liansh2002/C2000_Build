#ifndef __CONFIG_H
#define __CONFIG_H

//#define USE_LNINT                           //�Ƿ�ʹ��LNINT³��Э��
#define dUSE_LNINT_POWER                         //³�ܺ㹦��
//#define dNOT_CAN                            //��ʹ��CAN���
//#define __SIGNAL_SAMPLE                   //��·AD����(DCVOLT2, AN0 ��Ϊ���ֲ���)

//#define __DEBUG

#define dFREQ_TO_LOUT(f) ((3200000UL + (f / 2)) / f)    //Ƶ��ֱ��ת����ֵ
#define dQ10_DEAD_CURR_THR 3584l            //�ڹ������л��У�������ֵ
#define dQ10_DEAD_VOLT_THR 460800l          //�ڹ������л��У���ѹ��ֵ

//����Ƶ�����
#define dPERIOD_80K 40000U
#define dPERIOD_70K 45714U
#define dPERIOD_MAX dPERIOD_70K

//PFC��ѹ���
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

//������
#define CURR_RATE 20480                     //�����20A
#define dQ10_VOUT_MIN (99l << 10)           //��С�����ѹ
#define dQ10_VOUT_MAX (750l << 10)          //��������ѹ
#define dQ10_VOUT_INIT (300l << 10)         //Ĭ�������ѹ(��ʼ��ѹ)
#define dQ10_VOUT_FT_DEF (720l <<10)        //Ĭ�ϸ����ѹ
#define dQ10_OVP_DEF (760l << 10)           //OVPĬ��ֵ
#define dQ10_OVP_MAX (760l << 10)           //OVP���ֵ
#define dQ10_OVP_MIN (300l << 10)           //OVP��Сֵ

//DCDC����
#define dPERIOD_OUT_MIN 0x23C0l             //������Сֵ(Ƶ�ʴ�ֵ0x23C0 = 350KHz)
#define dDEAD_ADJ1_MAX 410                  //���ڿ����ڹܺ���ܼ�ռ�ձȲ�ֵ(��ֵԽ���ڹ�ռ�ձ�ԽС)
#define dDEAD_ADJ_MIN_NF 14                 //���ڿ���PWM2��PWM3֮�����λ��(�ڹ�����)����СΪ14
#define dDEAD_ADJ_MIN_HF 30
#define dRESONANCE_POINT 0x5355             //г���

#if defined(dUSE_LNINT_POWER)
     //������
    #define dQ10_CURR_LIMIT_MAX 1920l           //���������
    #define dQ10_CURR_LIMIT_MIN 100l            //��С������
    #define dQ10_POWER_LIMIT_MAX 1026l          //����޹��ʵ�
    #define dQ10_POWER_LIMIT_MIN 238l           //��С�޹��ʵ�
    #define dQ10_CURR_LIMIT_VREF (765l << 10)   //�����ѹ����ֵ

    //�����ѹ����������
    #define dQ10_LOW_VOLT_TOP (409088l)         //�͵�ѹ��������� ���ǲ�������330V����329.5V
    #define dQ10_LOW_VOLT_BOTTOM (204288l)      //�͵�ѹ��������� ���ǲ�������300V����299.5V
    #define dQ10_CURR_LIMIT_VOLT_MIN 1715l      //�͵�ѹ����������
    #define dQ10_CURR_LIMIT_VOLT_STEP (((dQ10_CURR_LIMIT_MAX - dQ10_CURR_LIMIT_VOLT_MIN) << 20) / (dQ10_LOW_VOLT_TOP - dQ10_LOW_VOLT_BOTTOM))    //((330-300)/(2457-1920))*1024

    //������
    #define dQ10_POWER_GAIN_CURR_MIN 30720l
    #define dQ10_CURR_MAX ((dQ10_CURR_LIMIT_MAX * CURR_RATE) >> 10)
#else
    //������
    #define dQ10_CURR_LIMIT_MAX 2304l           //���������
    #define dQ10_CURR_LIMIT_MIN 100l            //��С������
    #define dQ10_POWER_LIMIT_MAX 1026l          //����޹��ʵ�
    #define dQ10_POWER_LIMIT_MIN 238l           //��С�޹��ʵ�
    #define dQ10_CURR_LIMIT_VREF (765l << 10)   //�����ѹ����ֵ

    //�����ѹ����������
    #define dQ10_LOW_VOLT_TOP (337408l)         //�͵�ѹ��������� ���ǲ�������330V����329.5V
    #define dQ10_LOW_VOLT_BOTTOM (306688l)      //�͵�ѹ��������� ���ǲ�������300V����299.5V
    #define dQ10_CURR_LIMIT_VOLT_MIN 1920l      //�͵�ѹ����������
    #define dQ10_CURR_LIMIT_VOLT_STEP (((dQ10_CURR_LIMIT_MAX - dQ10_CURR_LIMIT_VOLT_MIN) << 20) / (dQ10_LOW_VOLT_TOP - dQ10_LOW_VOLT_BOTTOM))    //((330-300)/(2457-1920))*1024

    //������
    #define dQ10_POWER_GAIN_CURR_MIN 35840l
    #define dQ10_CURR_MAX ((dQ10_CURR_LIMIT_MAX * CURR_RATE) >> 10)
#endif

//�������
#define dQ10_AMB_LIMIT_MAX (70l << 10)      //��������
#define dQ10_AMB_LIMIT_MIN (53l << 10)      //���½�����ʼ��

//�������
#define dQ10_POWER_GAIN (200l << 10)        //��������ֵ

//����
#define ELEMENTS_IN(array) __elements_in_##array                                            //����һ������
#define ELEMENTS_IN_DEF(array) const unsigned int __elements_in_##array = sizeof(array)     //����һ�����������ʼֵΪ��������ĳ���
#define ELEMENTS_IN_DECLARE(array) extern const unsigned int __elements_in_##array          //����ǰһ������ı���

//³��Э�����
#define dLNINT_ADDR_MIN 1
#define dLNINT_ADDR_MAX 127
#define dLNINT_GROUP_MIN 1
#define dLNINT_GROUP_MAX 126
#define dLNINT_BAUD_MIN 50
#define dLNINT_BAUD_MAX 500
#define dLNINT_BAUD_DEF 250

#define dEEPROM_ADDR_ADDR 176                   //������ַ
#define dEEPROM_GROUP_ADDR 180                  //�����
#define dEEPROM_BAUD_ADDR 184                   //ͨѶ����
#define dEEPROM_SHARE_ADDR 188                  //��������

#define dEEPROM_ADDR_FLAG 0x1000
#define dEEPROM_GROUP_FLAG 0x2000
#define dEEPROM_BAUD_FLAG 0x4000
#define dEEPROM_SHARE_FLAG 0x8000

//״̬���붨��(uiMdlStatus)
#define dDCDC_OV        0x0001          //�����ѹ
#define	dOVER_TEMP      0x0002          //����
#define	dFAN_FAIL       0x0004          //���ȹ���
#define dMDL_OFF        0x0008          //MDL�ػ�
#define dPSM_OFF        0x0010          //ͨѶ�ػ�
//#define dDIO_FAIL       0x0020        //δʹ��
//#define dCOMM_FAIL      0x0040        //δʹ��
//#define dAC_FAIL        0x0080        //δʹ��
#define dSTATUS_PFC_FAIL 0x0100         //PFC����
#define	dAVGCUR_FAIL    0x0200          //�����쳣
#define dSTATUS_CAN_FAIL 0x0400         //CANͨѶ�쳣    
#define dSHORT_CIRCUIT  0x0800          //�����·
#define dEPROM_FAIL     0x1000          //EEPROMֵ����(ID_ERROR)
#define	dSCI_FAIL       0x2000          //SCIͨѶ�쳣    
#define dSTATUS_GLED_FLASH 0x4000       //ͨѶ�̵���˸
#define dDCDC_HVSD      0x8000          //�����ѹ����

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

