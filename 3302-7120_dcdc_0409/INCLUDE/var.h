#ifndef __VAR_H
#define __VAR_H

//��������
void init_can_extend(void);
void lnint_stack(void);

//ʱ��
extern unsigned int uiTIME;

//��־λ
extern union{
    unsigned int Symbol;
    struct{
        unsigned lock:1;
        unsigned share_on:1;            //1 ��������� 0 �رվ���(Ĭ��ֵ)

        unsigned broadcast:1;           //1 �㲥ָ��
    }bit;
}Flag;


//³��Э�����
extern unsigned int uiAddrConst;        //�̻���ģ���ַ
extern unsigned int uiGroupConst;       //�̻��ķ����
extern unsigned int uiGroupTemp;        //��ʱ�ķ����
extern unsigned int uiBaudConst;        //�̻���ͨѶ����

//����
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

