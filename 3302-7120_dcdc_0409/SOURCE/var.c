#include "config.h"

//ʱ��
unsigned int uiTIME = 0;

union{
    unsigned int Symbol;
    struct{
        unsigned lock:1;
        unsigned share_on:1;            //1 ��������� 0 �رվ���(Ĭ��ֵ)

        unsigned broadcast:1;           //1 �㲥ָ��
    }bit;
}Flag = {0};

//³��Э�����
unsigned int uiAddrConst;               //�̻���ģ���ַ
unsigned int uiGroupConst;              //�̻��ķ����
unsigned int uiGroupTemp;               //��ʱ�ķ����
unsigned int uiBaudConst;               //�̻���ͨѶ����


