#ifdef USE_LNINT

#include "IQmathLib.h"
#include "DSP280x_Device.h"
#include "config.h"
#include "lnint_can.h"
#include "var.h"

//���ñ���
static const unsigned int __INDEX[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static MSGID_LNINT_t t_msgid_receive_reg, t_msgid_transmit_reg;         //�շ���������ID�Ĵ���
static unsigned int ui_30s_count = 0;                                   //30�����ڼ���(B2-2)
static unsigned int ui_3s_count = 0;                                    //3�����ڼ���(B2-1��B3)
static union{
    unsigned int symbol;
    struct{
        unsigned vin_ov:1;      //1 �����ѹ
        unsigned vin_uv:1;      //1 ����Ƿѹ
        unsigned vout_ov:1;     //1 �����ѹ
        unsigned vout_uv:1;     //1 ���Ƿѹ
        
        unsigned oc:1;          //1 �������
        unsigned ot:1;          //1 ����
        unsigned fan:1;         //1 ����
        unsigned other:1;       //1 �����澯
        
        unsigned empty:8;       //��8λ��ʹ��
    }bit;
}warn_lnint = {0};
static union{
    unsigned int symbol;
    struct{
        unsigned share:1;       //1 ��������
        unsigned derate:1;      //1 �����
        
        unsigned empty:14;      //  δʹ��
    }bit;
}status_lnint = {0};
queue_t lnint_answer_q, lnint_auto_q;   //�������ݶ���

/******************************************
baud = SYSCLKOUT / (BRPREG + 1) * ((TSEG1REG + 1) + (TSEG2REG + 1) + 1)
Tsjwͬ����ת�ο�ȣ�Tsjw = Tscl * (2 * SJW[1] + 1 * SJW[0]).���Ϊ4������С��TSEG1�Լ�TSEG2��
******************************************/
void reset_can_baud(unsigned int ui_can_baud)
{
    struct ECAN_REGS ECanaShadow;                   //���������ֲᣬΪ��ֹCANģ����ֲ���Ԥ����Ϊ����CANģ���д��Ӧʹ��λ����

    ECanaShadow.CANBTC.all = 0;
    ECanaShadow.CANMD = ECanaRegs.CANMD;
    ECanaShadow.CANME = ECanaRegs.CANME;            //����ʹ�ܵ�����
    switch(ui_can_baud)
    {
        case 50:    //50KHz������
        {
            ECanaShadow.CANBTC.bit.BRPREG = 124;    //Tq = 1.25uS    
            ECanaShadow.CANBTC.bit.TSEG1REG = 10;
            ECanaShadow.CANBTC.bit.TSEG2REG = 3;
            ECanaShadow.CANBTC.bit.SAM = 1;         //0:���β�����1:���β���(ֻҪ��ƵBRP>4�Ϳ���ʹ��)
            ECanaShadow.CANBTC.bit.SJWREG = 2;
        }break;

        case 100:   //100KHz������
        {
            ECanaShadow.CANBTC.bit.BRPREG = 74;     //Tq = 0.75uS    
            ECanaShadow.CANBTC.bit.TSEG1REG = 10;
            ECanaShadow.CANBTC.bit.TSEG2REG = 3;
            ECanaShadow.CANBTC.bit.SAM = 1;         //0:���β�����1:���β���(ֻҪ��ƵBRP>4�Ϳ���ʹ��)
            ECanaShadow.CANBTC.bit.SJWREG = 2;
        }break;

        case 125:   //125KHz������
        {
            ECanaShadow.CANBTC.bit.BRPREG = 49;     //Tq = 0.5uS    
            ECanaShadow.CANBTC.bit.TSEG1REG = 10;
            ECanaShadow.CANBTC.bit.TSEG2REG = 3;
            ECanaShadow.CANBTC.bit.SAM = 1;         //0:���β�����1:���β���(ֻҪ��ƵBRP>4�Ϳ���ʹ��)
            ECanaShadow.CANBTC.bit.SJWREG = 2;
        }break;

        case 250:   //250KHz������
        {
            ECanaShadow.CANBTC.bit.BRPREG = 24;     //Tq = 0.25uS    
            ECanaShadow.CANBTC.bit.TSEG1REG = 10;
            ECanaShadow.CANBTC.bit.TSEG2REG = 3;
            ECanaShadow.CANBTC.bit.SAM = 1;         //0:���β�����1:���β���(ֻҪ��ƵBRP>4�Ϳ���ʹ��)
            ECanaShadow.CANBTC.bit.SJWREG = 2;
        }break;

        case 500:   //500KHz������
        {
            ECanaShadow.CANBTC.bit.BRPREG = 9;      //Tq = 0.1uS    
            ECanaShadow.CANBTC.bit.TSEG1REG = 13;
            ECanaShadow.CANBTC.bit.TSEG2REG = 4;
            ECanaShadow.CANBTC.bit.SAM = 1;         //0:���β�����1:���β���(ֻҪ��ƵBRP>4�Ϳ���ʹ��)
            ECanaShadow.CANBTC.bit.SJWREG = 2;
        }break;

        default:
        {
        }return;    //�����κδ���
    }
    ECanaRegs.CANMD.all = 0;
    ECanaRegs.CANME.all = 0;                    //�ȹر��������䣬���л���ɴ�
    ECanaRegs.CANRMP.all = 0xFFFFFFFF;          //������н�����ɱ�־
    
    EALLOW;
    ECanaRegs.CANMC.bit.CCR = 1;
    while(!ECanaRegs.CANES.bit.CCE);            //��CCEΪ1ʱ��CANBTC��������ɹ�(����CCE��Ҫ��⵽RX�Ÿߵ�ƽ������1��������Ҫ�ȴ�)

    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

    ECanaRegs.CANMC.bit.CCR = 0;                //����CAN����
    while(ECanaRegs.CANES.bit.CCE);
    EDIS;

    ECanaRegs.CANME.all = ECanaShadow.CANME.all;    //�ָ���������
    ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;    //�ָ�������/��ѡ��
    
}

/**************************************
CAN��ʼ�����䣬����LNINTЭ�鴴��
**************************************/
void init_can_extend(void)
{
    t_msgid_receive_reg.p = (void *)&ECanaMboxes.MBOX2.MSGID;
    t_msgid_transmit_reg.p = (void *)&ECanaMboxes.MBOX6.MSGID;
    
    //����LNINTЭ������
    t_msgid_receive_reg.bit->PRIORITY = 0x18;
    t_msgid_receive_reg.bit->IDE = 1;           //��չ֡
    t_msgid_receive_reg.bit->AME = 1;           //��������ʹ��
    t_msgid_receive_reg.bit->AAM = 0;           //�����Զ�Ӧ��
    
    t_msgid_transmit_reg.bit->PRIORITY = 0x18;
    t_msgid_transmit_reg.bit->IDE = 1;          //��չ֡
    t_msgid_transmit_reg.bit->AME = 1;          //��������ʹ��
    t_msgid_transmit_reg.bit->AAM = 0;          //�����Զ�Ӧ��

    ECanaRegs.CANME.all = 0x00000033 | 0x00000044;  //��2��6
    ECanaRegs.CANMD.all = 0x00000003 | 0x00000004;  //mailbox2���ڽ���LNINTЭ��
    
    //CAN���ʵ���
    reset_can_baud(uiBaudConst);                    //����CAN����

    //���г�ʼ��
    queue_init(&lnint_answer_q);
    queue_init(&lnint_auto_q);
}
/**************************************
lnintģ�����ϱ�(B2-1)
**************************************/
void lnint_analog_b21(PACK_LNINT_t *data_t)
{
    unsigned int ui_tmp;
    
    data_t->u16_data[0] = 0x0100;
    
    //A���ѹ
    ui_tmp = ((iq10Vaca.lData * 10) + 512) >> 10;
    data_t->u16_data[0] |= (ui_tmp & 0x00FF);
    data_t->u16_data[1] = (ui_tmp << 8);
    
    //B���ѹ
    ui_tmp = ((iq10Vacb.lData * 10) + 512) >> 10;
    data_t->u16_data[1] |= (ui_tmp & 0x00FF);
    data_t->u16_data[2] = (ui_tmp << 8);
    
    //C���ѹ
    ui_tmp = ((iq10Vacc.lData * 10) + 512) >> 10;
    data_t->u16_data[2] |= (ui_tmp & 0x00FF);
    data_t->u16_data[3] = (ui_tmp << 8);

    data_t->u16_data[3] |= 0xFF00;
}
/**************************************
lnintģ�����ϱ�(B2-2)
**************************************/
void lnint_analog_b22(PACK_LNINT_t *data_t)
{
    unsigned int ui_tmp;

    data_t->u16_data[0] = 0x0200;

    //�����ѹ
    ui_tmp = ((iq10MdlVolt.lData * 10) + 512) >> 10;
    data_t->u16_data[0] |= (ui_tmp & 0x00FF);
    data_t->u16_data[1] = (ui_tmp << 8);
    
    //�������
    ui_tmp = ((iq10CurrDisp.lData * 10) + 512) >> 10;
    data_t->u16_data[1] |= (ui_tmp & 0x00FF);
    data_t->u16_data[2] = (ui_tmp << 8);
    
    //�����¶�
    ui_tmp = ((iq10TempCtrl.lData * 10) + 512512) >> 10;        //�ͻ�������50��ƫ�ã��ټ�512������������
    data_t->u16_data[2] |= (ui_tmp & 0x00FF);
    data_t->u16_data[3] = (ui_tmp << 8);

    data_t->u16_data[3] |= 0xFF00;
}

/**************************************
lnint�澯���ϱ�(B3)
**************************************/
unsigned int lnint_warn_b3(PACK_LNINT_t *data_t)
{
    static unsigned int warn_back = 0;
    static unsigned int status_back = 0;
    
    //�澯���
        //�����ѹ
        
        //����Ƿѹ
        
        //�����ѹ
    if(uiMdlStatus & (dDCDC_OV | dDCDC_HVSD))
    {
        warn_lnint.bit.vout_ov = 1;
    }
    else
    {
        warn_lnint.bit.vout_ov = 0;
    }
        //���Ƿѹ
    
        //�����· 
    if(uiMdlStatus & dSHORT_CIRCUIT)
    {
        warn_lnint.bit.oc = 1;
    }
    else
    {
        warn_lnint.bit.oc = 1;
    }
        //����
    if(uiMdlStatus & dOVER_TEMP)
    {
        warn_lnint.bit.ot = 1;
    }
    else
    {
        warn_lnint.bit.ot = 0;
    }
        //����
    if(uiMdlStatus & dFAN_FAIL)
    {
        warn_lnint.bit.fan = 1;
    }
    else
    {
        warn_lnint.bit.fan = 0;
    }
    
    //״̬���
    if(Flag.bit.share_on)                       //�������
    {
        status_lnint.bit.share = 1;
    }
    else                                        //��ֹ����
    {
        status_lnint.bit.share = 0;
    }
    if(ucLimitStatus & 0x06)                    //�¶���������������
    {
        status_lnint.bit.derate = 1;
    }
    else
    {
        status_lnint.bit.derate = 0;
    }

    //д����
    data_t->u16_data[0] = uiGroupConst;
    if(0 == (uiMdlStatus & (dPSM_OFF | dMDL_OFF)))  //��ǰ����
    {
        data_t->u16_data[0] |= 0x0100;
    } 
    data_t->u16_data[1] = status_lnint.symbol;  //״̬λ
    data_t->u16_data[2] = 0xFFFF;               //δʹ��
    data_t->u16_data[3] = warn_lnint.symbol;    //�澯λ

    //״̬���
    if((warn_back != warn_lnint.symbol) || (status_back != status_lnint.symbol))        //�з���״̬����
    {
        //���±�������
        warn_back = warn_lnint.symbol;
        status_back = status_lnint.symbol;
        return(1);
    }
    return(0);
}


/**************************************
lnintЭ�鴦��
**************************************/
#define dLNINT_DATA lnint_pack.u16_data             //��ʹ�ú�dGET_BYTE��dGET_WORDǰ�����ȶ���������Դ    
void lnint_process(PACK_LNINT_t lnint_pack)
{
    ubitfloat iq10SetVoltTmp, iq10SetLimitTmp;
    unsigned int ui_control;
    long l_tmp;
    PACK_LNINT_t answer_t;                          //Ӧ�����ݻ���

    Flag.bit.broadcast = 0;                         //Ĭ�ϰ���ַ����ָ��
    if(lnint_pack.target & 0x0080)                  //��������
    {
        if((lnint_pack.target != (uiGroupTemp | 0x0080)) && (0x00FF != lnint_pack.target))
        {
            return;
        }
        Flag.bit.broadcast = 1;                     //�㲥(��������ָ��)
    }
    if(lnint_pack.target != uiAddrConst)            //����ַ����
    {
        return;
    }
    //Ӧ������ǰ�ô���
    answer_t.cmd = lnint_pack.cmd | 0x0010;         //ģ��Ӧ��Ϊ0xBn
    answer_t.source = uiAddrConst;
    answer_t.target = lnint_pack.source;
    answer_t.u16_data[0] = 0xFFFF;
    answer_t.u16_data[1] = 0xFFFF;
    answer_t.u16_data[2] = 0xFFFF;
    answer_t.u16_data[3] = 0xFFFF;
    
    //Э�鴦��
    switch(lnint_pack.cmd)
    {
        case 0xA1:                                      //ֱ�ӹ̻���ַ
        {
            if((dLNINT_ADDR_MAX >= dLNINT_BYTE(0)) && (dLNINT_ADDR_MIN <= dLNINT_BYTE(0)))
            {
                if(dLNINT_BYTE(0) != uiAddrConst)
                {
                    uiAddrConst = dLNINT_BYTE(0);
                    uiEepromFlag |= dEEPROM_ADDR_FLAG;  //�б仯��д��EEPROM
                }
                answer_t.u16_data[0] = dSET_SUCCESS;    //���óɹ�
            }
            else
            {
                answer_t.u16_data[0] = dSET_FAILURE;    //����ʧ��
            }
            answer_t.source = uiAddrConst;              //���»ش��ı�����ַ
            if(!Flag.bit.broadcast)
            {
                queue_put_item(&lnint_answer_q, &answer_t); //���뷢�Ͷ���
            }    
        }break;

        case 0xA2:                                      //�̻������(��Ӧ��)
        {
            if((dLNINT_GROUP_MAX >= dLNINT_BYTE(0)) && (dLNINT_GROUP_MIN <= dLNINT_BYTE(0)))
            {
                if(dLNINT_BYTE(0) != uiGroupConst)
                {
                    uiGroupConst = dLNINT_BYTE(0);
                    uiEepromFlag |= dEEPROM_GROUP_FLAG;
                }
                uiGroupTemp = uiGroupConst;             //��ʱ���ͬ�����
            }
        }break;

        case 0xA3:
        case 0xA4:                                      //����(�������)(��Ӧ��)
        {
            ui_control = 0;                             //Ĭ��Ϊ����Ա����Ŀ���
            
            iq10SetVoltTmp.lData = dLNINT_WORD(0);      //��ȡ��ѹ����ֵ
            iq10SetVoltTmp.lData = ((iq10SetVoltTmp.lData << 10) + 5) / 10;     //תIQ10��ʽ
            
            iq10SetLimitTmp.lData = dLNINT_WORD(1);     //��ȡ��������ֵ
            iq10SetLimitTmp.lData = ((iq10SetLimitTmp.lData << 10) + 5) / 10;   //תIQ10��ʽ�ٷֱ�ֵ
            iq10SetLimitTmp.lData = _IQ10div(iq10SetLimitTmp.lData, CURR_RATE);
            
            l_tmp = dLNINT_DATA[2];
            l_tmp <<= 16;
            l_tmp |= dLNINT_DATA[3];                    //��ȡ���ʶ����

            if(0xFF != lnint_pack.target)               //��Ⱥ�أ���Ա���
            {
                ui_control = 1;
            }
            else if(l_tmp & (1Ul << ((uiGroupTemp - 1) & 31)))   //Ⱥ�أ�����Ա���
            {
                ui_control = 1;
            }
            
            if(1 == ui_control)                         //��Ҫ����
            {
                //��ѹ��Χ���
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
                //������Χ���
                if(iq10SetLimitTmp.lData > dQ10_CURR_LIMIT_MAX)
                {
                    iq10SetLimitTmp.lData = dQ10_CURR_LIMIT_MAX;
                }
                else if(iq10SetLimitTmp.lData < dQ10_CURR_LIMIT_MIN)
                {
                    iq10SetLimitTmp.lData = dQ10_CURR_LIMIT_MIN;
                }
                
                //����ʵ��ֵ
                //���ڴ�ָ��û�лش����ݣ���Э��Ҫ������ֵ������/����ʱ������ִֵ��
                iq10SetVolt.lData = iq10SetVoltTmp.lData;
                iq10SetLimit.lData = iq10SetLimitTmp.lData;
                
                if(0xA3 == lnint_pack.cmd)          //����
                {
                    uiMdlStatus &= ~(dMDL_OFF | dPSM_OFF);
                    uwOnOffDCDC = 0x0;
                }
                else                                //�ػ�
                {
                    uiMdlStatus |= (dMDL_OFF | dPSM_OFF);
                    uiActionReady = 0;
                    uwOnOffDCDC = 0x01;
                    iq10SetVoltTrue.lData = dQ10_VOUT_INIT;
                }
            }
        }break;

        case 0xA5:                                  //����ͨѶ����
        {
            if((dLNINT_BAUD_MAX >= dLNINT_WORD(0)) && (dLNINT_BAUD_MIN <= dLNINT_WORD(0)))
            {
                if(dLNINT_WORD(0) != uiBaudConst)
                {
                    uiBaudConst = dLNINT_WORD(0);   //д�룬����Ҫ�ϵ����������Ч
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

        case 0xA6:                                  //��ʱ���ģ�����(��Ӧ��)
        {
            if(0 != dLNINT_BYTE(1))                 //�ָ���ʱ�����Ϊ�̻�ֵ
            {
                uiGroupTemp = uiGroupConst;
            }
            else if((dLNINT_ADDR_MAX >= dLNINT_BYTE(0)) && (dLNINT_ADDR_MIN <= dLNINT_BYTE(0)))
            {
                uiGroupTemp = dLNINT_BYTE(0);
            }
        }break;

        case 0xA7:                                  //��ѯģ��������Ϣ
        {
            if(0x01 == dLNINT_BYTE(0))              //�ش���ʱ����(��Ӧ��)
            {
                ui_30s_count = 0;
                ui_3s_count = 0;
            }
            else if(0x01 == dLNINT_BYTE(1))         //B2-01֡
            {
                lnint_analog_b21(&answer_t);
                if(Flag.bit.broadcast)
                {
                    queue_put_item(&lnint_answer_q, &answer_t);
                }    
            }
            else if(0x01 == dLNINT_BYTE(2))         //B2-02֡
            {
                lnint_analog_b22(&answer_t);
                if(Flag.bit.broadcast)
                {
                    queue_put_item(&lnint_answer_q, &answer_t);
                }    
            }
            else if(0x01 == dLNINT_BYTE(3))         //B3֡
            {
                lnint_warn_b3(&answer_t);
                if(Flag.bit.broadcast)
                {
                    queue_put_item(&lnint_answer_q, &answer_t);
                }    
            }
            
        }break;

        case 0xA8:                                  //��ѯ��Լ�汾�ż�����ֵ
        {
            answer_t.cmd = 0x00B8;                  //B8����
            answer_t.u16_data[0] = dLNINIT_VER;                                             //��Լ�汾
            answer_t.u16_data[1] = dQ10_TO_LNINT(_IQ10mpy(dQ10_CURR_LIMIT_MIN, CURR_RATE)); //��С��������ֵ
            answer_t.u16_data[2] = dQ10_TO_LNINT(_IQ10mpy(dQ10_CURR_LIMIT_MAX, CURR_RATE)); //�����������ֵ
            answer_t.u16_data[3] = dQ10_TO_LNINT(_IQ10mpy(iq10MdlLimit.lData, lCurrRate));  //������������ֵ
            if(!Flag.bit.broadcast)
            {
                queue_put_item(&lnint_answer_q, &answer_t);
            }    

            answer_t.cmd = 0x00B9;                  //B9����
            answer_t.u16_data[0] = dQ10_TO_LNINT(dQ10_VOUT_MIN);                            //��С�����ѹ
            answer_t.u16_data[1] = dQ10_TO_LNINT(dQ10_VOUT_MAX);                            //��������ѹ
            answer_t.u16_data[2] = dQ10_TO_LNINT(dQ10_AMB_LIMIT_MIN);                       //�����(����)
            answer_t.u16_data[3] = dSOFT_VER_HEX;                                           //����汾
            if(!Flag.bit.broadcast)
            {
                queue_put_item(&lnint_answer_q, &answer_t);
            }    
        }break;

        case 0xAF:                                  //����
        {
            if(0 == dLNINT_BYTE(0))                 //��ֹ����
            {
                if(Flag.bit.share_on)
                {
                    Flag.bit.share_on = 0;
                    uiEepromFlag |= dEEPROM_SHARE_FLAG;
                    answer_t.u16_data[0] = 0xFF00;  //�ش���ֹ����
                }
            }
            else                                    //�������(��ָ��ش�ֻ�п�/������״̬��û������ʧ�ܵ�ѡ��)
            {
                if(!Flag.bit.share_on)
                {
                    Flag.bit.share_on = 1;
                    uiEepromFlag |= dEEPROM_SHARE_FLAG;
                    answer_t.u16_data[0] = 0xFF01;  //�ش��������
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

    //ǰ�ô���
    auto_t.source = uiAddrConst;
    auto_t.target = uiGroupTemp;                        //�����ϱ�����Ŀ���ַΪ���������

    //�Զ���������
    ui_30s_count++;
    ui_3s_count++;
    if(lnint_warn_b3(&auto_t))                          //�б仯�����
    {
        auto_t.cmd = 0xB3;
        queue_put_item(&lnint_auto_q, &auto_t);         //�ͷ��Ͷ���
    }
    else if(ui_30s_count >= 3000)                       //B2-02
    {
        ui_30s_count = 0;
        auto_t.cmd = 0xB2;
        lnint_analog_b22(&auto_t);
        queue_put_item(&lnint_auto_q, &auto_t);         //�ͷ��Ͷ���
    }
    else if(ui_3s_count >= 300)                         //B2-01,B3
    {
        ui_3s_count = 0;
        auto_t.cmd = 0xB2;
        lnint_analog_b21(&auto_t);
        queue_put_item(&lnint_auto_q, &auto_t);         //�ͷ��Ͷ���
        
        lnint_warn_b3(&auto_t);                         //B3����3�붨ʱ����
        queue_put_item(&lnint_auto_q, &auto_t);         //�ͷ��Ͷ���
    }
    else
    {
        return;
    }
}

/**************************************
lnintЭ��ջ
**************************************/
void lnint_stack(void)
{
    struct ECAN_REGS ECanaShadow;
    PACK_LNINT_t lnint_data_t;
    
    ECanaShadow.CANRMP.all = ECanaRegs.CANRMP.all;
    ECanaShadow.CANTA.all = ECanaRegs.CANTA.all;        //��������״̬
    
    if(ECanaShadow.CANRMP.bit.RMP2)                     //����2������
    {
        lnint_data_t.cmd = t_msgid_receive_reg.bit->CMD & 0x00FF;
        lnint_data_t.source = t_msgid_receive_reg.bit->GS & 0x00FF;
        lnint_data_t.target = t_msgid_receive_reg.bit->SA & 0x00FF;
        lnint_data_t.u16_data[0] = ECanaMboxes.MBOX2.MDL.word.HI_WORD;      //bit[31:16]--byte0,byte1
        lnint_data_t.u16_data[1] = ECanaMboxes.MBOX2.MDL.word.LOW_WORD;     //bit[15:00]--byte2,byte3
        lnint_data_t.u16_data[2] = ECanaMboxes.MBOX2.MDH.word.HI_WORD;      //bit[31:16]--byte4,byte5
        lnint_data_t.u16_data[3] = ECanaMboxes.MBOX2.MDH.word.LOW_WORD;     //bit[15:00]--byte6,byte7
        
        ECanaShadow.CANRMP.all = (1Ul << 2);
        ECanaRegs.CANRMP.all = ECanaShadow.CANRMP.all;  //���������ɱ�־

        lnint_process(lnint_data_t);                    //Э�鴦��
    }

    //Э���ж�ʱ����
    lnint_time();

    //��鷢�Ͷ��в�����
    //�����ȼ��Ӧ����У��ټ���Զ����Ͷ���
    if(!ECanaRegs.CANTA.bit.TA6 || ECanaRegs.CANTRS.bit.TRS6)
    {
        return;
    }
    if(0 == queue_get_item(&lnint_answer_q, &lnint_data_t) &&\
        0 == queue_get_item(&lnint_auto_q, &lnint_data_t))              //�ȼ��Ӧ�����
    {
        return;
    }
    ECanaRegs.CANME.all = (ECanaRegs.CANME.all & 0xFFFFFFBF);           //�ȹر�����6
    t_msgid_transmit_reg.bit->CMD = lnint_data_t.cmd;
    t_msgid_transmit_reg.bit->GS = lnint_data_t.target;
    t_msgid_transmit_reg.bit->SA = lnint_data_t.source;
    ECanaRegs.CANME.all = (ECanaRegs.CANME.all | 0x00000040);           //������6
    
    ECanaMboxes.MBOX2.MDL.word.HI_WORD = lnint_data_t.u16_data[0];      //bit[31:16]--byte0,byte1
    ECanaMboxes.MBOX2.MDL.word.LOW_WORD = lnint_data_t.u16_data[1];     //bit[15:00]--byte2,byte3
    ECanaMboxes.MBOX2.MDH.word.HI_WORD = lnint_data_t.u16_data[2];      //bit[31:16]--byte4,byte5
    ECanaMboxes.MBOX2.MDH.word.LOW_WORD = lnint_data_t.u16_data[3];     //bit[15:00]--byte6,byte7
    
    ECanaRegs.CANTRS.all = (ECanaRegs.CANTRS.all | 0x00000040);         //��������
}


#else
void init_can_extend(void)
{
}
void lnint_stack(void)
{
}
#endif





