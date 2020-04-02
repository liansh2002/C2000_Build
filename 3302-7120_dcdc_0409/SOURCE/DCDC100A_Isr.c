//#########################################################

//######################################################## 


#include "DSP280x_Device.h"
#include "config.h"                 //2018-10-22
#include "var.h"                    //2018-10-23

#pragma CODE_SECTION(dcdc_isr, "ramfuncs");
//
extern ubitfloat    iq10MdlAvgCurr;
extern ubitfloat	iq10MdlCurr;
extern signed   int     AvgCurr;
extern unsigned int		uiMdlStatus;
extern unsigned int		uiActionReady;
extern unsigned int     VASAMPLE_INV;
extern unsigned int     uiSelfChkEn; //
extern unsigned int      il732ref;   //2017 20kw
extern unsigned int      il607ref;
extern unsigned int      il550ref;
extern unsigned int      il420ref;
extern unsigned int      il430ref;
extern unsigned int      il360ref;
extern unsigned int      il350ref;
extern unsigned int      il310ref;
extern unsigned int      il303ref;
extern unsigned int      uiAdjFlag;
extern signed int       lCurr8A;
extern signed int       lCurr10A;
extern signed   int     AvgCurr;
#define		MEP_SF							55*256		//
#define		DCDC_PWMMIN					110*64		//
#define		DCDC_VOLTMOD_CURRLIM			3500*8		//           
#define    ui_Dcdc_Curr_Out0min             50 

/***************************************************************************
*    Function Name: 	
***************************************************************************/ 
interrupt void dcdc_isr() //cpu  
{
    signed int			Dcdc_Tmp,Dcdc_Tmp1;
    signed long			Dcdc_TmpLong; 
    unsigned long		Dcdc_uTmpLong;

    uiTIME++;                   //2018-11-02
    	
	Dcdcisr.i_Dcdc_Curr_Err2 = Dcdcisr.i_Dcdc_Curr_Err1;
 	Dcdcisr.i_Dcdc_Curr_Err1 = Dcdcisr.i_Dcdc_Curr_Err0;

 	Dcdcisr.ui_Dcdc_Curr_Out2 = Dcdcisr.ui_Dcdc_Curr_Out1;
 	Dcdcisr.ui_Dcdc_Curr_Out1 = Dcdcisr.ui_Dcdc_Curr_Out0;

	Dcdcisr.ui_Dcdc_Run_Mode &= 0xfffc;
	/*if (Dcdcisr.ui_Dcdc_Power_Lim>=Dcdcisr.ui_Dcdc_Curr_Ref)
	{
		Dcdc_Tmp = Dcdcisr.ui_Dcdc_Curr_Ref;
		Dcdcisr.ui_Dcdc_Run_Mode |= 0x0002;        
	}
	else
	{
		Dcdc_Tmp = Dcdcisr.ui_Dcdc_Power_Lim;
		Dcdcisr.ui_Dcdc_Run_Mode |= 0x0001;    
	}*/
	//2018-11-16 Ôö¼Ó»Ø²î
	if((Dcdcisr.ui_Dcdc_Curr_Ref + 100) < Dcdcisr.ui_Dcdc_Power_Lim)
	{
		Dcdc_Tmp = Dcdcisr.ui_Dcdc_Curr_Ref;
		Dcdcisr.ui_Dcdc_Run_Mode |= 0x0002;        
	}
	else
	{
		Dcdc_Tmp = Dcdcisr.ui_Dcdc_Power_Lim;
		Dcdcisr.ui_Dcdc_Run_Mode |= 0x0001;    
	}
	//2018-11-16 end

   	Dcdcisr.i_Dcdc_Volt_Err2 = Dcdcisr.i_Dcdc_Volt_Err1;
 	Dcdcisr.i_Dcdc_Volt_Err1 = Dcdcisr.i_Dcdc_Volt_Err0;
 	Dcdcisr.ui_Dcdc_Volt_Out2 = Dcdcisr.ui_Dcdc_Volt_Out1;
 	Dcdcisr.ui_Dcdc_Volt_Out1 = Dcdcisr.ui_Dcdc_Volt_Out0;	 	 	
    
    if(Dcdcisr.ui_Dcdc_Volt_Ref_ctrol>=il607ref)
    {
        Dcdcisr.voref_dead_adj=0;
    }
    else if(Dcdcisr.ui_Dcdc_Volt_Ref_ctrol<=il303ref)//
    {
        Dcdcisr.voref_dead_adj=dDEAD_ADJ1_MAX;                  //20170522 tempup
    }
    else
    {
        Dcdc_Tmp1=(signed int)(((long)VASAMPLE_INV*157)>>12);   //20170522 tempup 
        Dcdcisr.voref_dead_adj=(unsigned int)(((long)((int)il607ref-Dcdcisr.ui_Dcdc_Volt_Ref_ctrol)*Dcdc_Tmp1) >> 12);
    }   
    if(Dcdcisr.voref_dead_adj>=dDEAD_ADJ1_MAX)                  //20170522 tempup
    {
        Dcdcisr.voref_dead_adj=dDEAD_ADJ1_MAX;                  //20170522 tempup
    }


    Dcdcisr.ui_Dcdc_Curr_Adc = AdcMirror.ADCRESULT5 + AdcMirror.ADCRESULT6 +AdcMirror.ADCRESULT8 +
    AdcMirror.ADCRESULT9 + AdcMirror.ADCRESULT10+ AdcMirror.ADCRESULT8 + AdcMirror.ADCRESULT6+ AdcMirror.ADCRESULT5;

   // 
////////////////////////////////////////////////////////////////////////////////////////////
    Dcdcisr.i_Dcdc_Curr_Err0 = Dcdc_Tmp + (signed int)Dcdcisr.ui_Dcdc_Curr_delta -(signed int)Dcdcisr.ui_Dcdc_Curr_Adc;  //Q15
    Dcdc_TmpLong = (long)Dcdcisr.ui_Dcdc_Curr_Out1*Dcdcisr.ui_Dcdc_Curr_K1 - (long)Dcdcisr.ui_Dcdc_Curr_Out2*Dcdcisr.ui_Dcdc_Curr_K2;
    Dcdc_TmpLong = Dcdc_TmpLong	+ (long)Dcdcisr.i_Dcdc_Curr_Err0 * Dcdcisr.ui_Dcdc_Curr_K3 	
            - (long)Dcdcisr.i_Dcdc_Curr_Err1 * Dcdcisr.ui_Dcdc_Curr_K4	+ (long)Dcdcisr.i_Dcdc_Curr_Err2 * Dcdcisr.ui_Dcdc_Curr_K5;	
    	
    
    if(Dcdcisr.currlimit_flag==0x01)  //
    {
        Dcdcisr.ui_Dcdc_Curr_Out0max=Dcdcisr.ui_Dcdc_Volt_Ref+80;
    }
    else
    {
        Dcdcisr.ui_Dcdc_Curr_Out0max=Dcdcisr.ui_Dcdc_Volt_Ref+160; 
    }
	 	
    if(Dcdc_TmpLong >= ( long)Dcdcisr.ui_Dcdc_Curr_Out0max<<CURR_SHIFT)//
    {
        Dcdcisr.ui_Dcdc_Curr_Out0 = Dcdcisr.ui_Dcdc_Curr_Out0max;	
    }
    else if(Dcdc_TmpLong <= (long)ui_Dcdc_Curr_Out0min<<CURR_SHIFT)
    {
        Dcdcisr.ui_Dcdc_Curr_Out0 = ui_Dcdc_Curr_Out0min;	
    }
    else
    {
        Dcdcisr.ui_Dcdc_Curr_Out0 = (unsigned int)(Dcdc_TmpLong >> CURR_SHIFT);
    }


    if ((Dcdcisr.ui_Dcdc_Curr_Adc>=Dcdcisr.ui_Dcdc_Curr_42A)&&(uiSelfChkEn==0))		//2017 20k
    {
        Dcdcisr.ui_Dcdc_Duty_Permit = 0x2941;
    }
    else
    {
        Dcdcisr.ui_Dcdc_Duty_Permit = Dcdcisr.ui_Dcdc_Duty_Ramp;
    }
    
    if(Dcdcisr.ui_Dcdc_Curr_Adc>=Dcdcisr.ui_Dcdc_Curr_35A)//2017 20k
    {
        Dcdcisr.ui_Dcdc_Curr_Adc_load=Dcdcisr.ui_Dcdc_Curr_35A;//2017 20k
    }
    else
    {
        Dcdcisr.ui_Dcdc_Curr_Adc_load=Dcdcisr.ui_Dcdc_Curr_Adc;
    }
    if ((uiActionReady>=3)&&uiSelfChkEn)                    //¿ª»úÍê³ÉÇÒµ¥»úÔËÐÐ        
    {				
        Dcdcisr.i_v_load= (((signed long)AvgCurr -Dcdcisr.ui_Dcdc_Curr_Adc_load) >> 7);	
    }
    else
    {
        Dcdcisr.i_v_load= 0;
    } 
   
            
    if(Dcdcisr.ui_Dcdc_Curr_Out0<=Dcdcisr.ui_Dcdc_Volt_Ref)
    {
        Dcdcisr.ui_Dcdc_Volt_Ref_ctrol=Dcdcisr.ui_Dcdc_Curr_Out0;
        Dcdcisr.ui_Dcdc_Run_Mode |= 0x0010;                 //ÏÞÁ÷Ä£Ê½
        Dcdcisr.currlimit_flag=0x01;       
    }	
    else
    {
        Dcdcisr.ui_Dcdc_Volt_Ref_ctrol=(signed int)Dcdcisr.ui_Dcdc_Volt_Ref+Dcdcisr.i_v_load;
        Dcdcisr.ui_Dcdc_Run_Mode = 0;                       //ÏÞÑ¹Ä£Ê½
        Dcdcisr.currlimit_flag=0x00;
    }
#if defined(__SIGNAL_SAMPLE)
    Dcdcisr.ui_Dcdc_Volt_Adc = (AdcMirror.ADCRESULT2+AdcMirror.ADCRESULT12) << 1;
#else
    Dcdcisr.ui_Dcdc_Volt_Adc = AdcMirror.ADCRESULT1+AdcMirror.ADCRESULT2+AdcMirror.ADCRESULT11+AdcMirror.ADCRESULT12;       //use Adc value
#endif
    Dcdcisr.i_Dcdc_Volt_Err0= (signed int)((signed int)Dcdcisr.ui_Dcdc_Volt_Ref_ctrol - (Dcdcisr.ui_Dcdc_Volt_Adc<<2) );    //Q15
    if((abs(Dcdcisr.i_Dcdc_Volt_Err0)>1182) && (Dcdcisr.currlimit_flag==0x00))
    {
        Dcdcisr.ui_Dcdc_Volt_K1 = 4209;
        Dcdcisr.ui_Dcdc_Volt_K2 = 113;
        Dcdcisr.ui_Dcdc_Volt_K3 = 13435;	
        Dcdcisr.ui_Dcdc_Volt_K4 = 20797;
        Dcdcisr.ui_Dcdc_Volt_K5 = 7859;
    }
    else if(Dcdcisr.ui_Dcdc_Volt_Ref_ctrol>=il310ref)
    {
        if(Dcdcisr.ui_Dcdc_Curr_Adc<=lCurr8A)
        {
            Dcdcisr.ui_Dcdc_Volt_K1 = 4209;
            Dcdcisr.ui_Dcdc_Volt_K2 = 113;
            Dcdcisr.ui_Dcdc_Volt_K3 = 16122;	
            Dcdcisr.ui_Dcdc_Volt_K4 = 24956;
            Dcdcisr.ui_Dcdc_Volt_K5 = 9431;	 	
        }	  	
        else if(Dcdcisr.ui_Dcdc_Curr_Adc>=lCurr10A)
        {  
            Dcdcisr.ui_Dcdc_Volt_K1 = 4209;
            Dcdcisr.ui_Dcdc_Volt_K2 = 113;
            Dcdcisr.ui_Dcdc_Volt_K3 = 5374;	
            Dcdcisr.ui_Dcdc_Volt_K4 = 8319;
            Dcdcisr.ui_Dcdc_Volt_K5 = 3144;
        }
    }  
    else if(Dcdcisr.ui_Dcdc_Volt_Ref_ctrol<=il303ref)
    {
        Dcdcisr.ui_Dcdc_Volt_K1 = 4209;
        Dcdcisr.ui_Dcdc_Volt_K2 = 113;
        Dcdcisr.ui_Dcdc_Volt_K3 = 24183;//7524;//10748;//16122;//
        Dcdcisr.ui_Dcdc_Volt_K4 = 37434;//11647;//16638;//24956;//
        Dcdcisr.ui_Dcdc_Volt_K5 = 14146;//4402;//6288;//9431;//
    }
    Dcdc_TmpLong = (long)Dcdcisr.ui_Dcdc_Volt_K1*Dcdcisr.ui_Dcdc_Volt_Out1 -(long)Dcdcisr.ui_Dcdc_Volt_K2*Dcdcisr.ui_Dcdc_Volt_Out2;
    Dcdc_TmpLong = Dcdc_TmpLong	+(long)Dcdcisr.i_Dcdc_Volt_Err0 * Dcdcisr.ui_Dcdc_Volt_K3 - (long)Dcdcisr.i_Dcdc_Volt_Err1 * Dcdcisr.ui_Dcdc_Volt_K4
                +(long)Dcdcisr.i_Dcdc_Volt_Err2 * Dcdcisr.ui_Dcdc_Volt_K5;
#ifndef __DEBUG           
    if ((Dcdcisr.ui_Dcdc_Volt_Adc<=600)&&(uiActionReady>=3)&&(Dcdcisr.ui_Dcdc_Short_Flag == 0))	// 600/4096/2*827=60V
    {
        Dcdcisr.ui_Dcdc_Duty_Short = 0x2941;
        Dcdcisr.ui_Dcdc_Short_Flag = 1;
        uiMdlStatus |= dSHORT_CIRCUIT;
    } 
#else
    Dcdcisr.ui_Dcdc_Short_Flag = 0;
#endif
    if ((Dcdcisr.ui_Dcdc_Duty_Short<Dcdcisr.ui_Dcdc_Duty_Permit)&&(Dcdcisr.ui_Dcdc_Short_Flag == 1))
    {
        Dcdcisr.ui_Dcdc_Duty_Permit = Dcdcisr.ui_Dcdc_Duty_Short;
	}	

 	//
    if(Dcdc_TmpLong >= (long)Dcdcisr.ui_Dcdc_Duty_Permit<<VOLT_SHIFT)
    {
        Dcdc_TmpLong = (long)Dcdcisr.ui_Dcdc_Duty_Permit<<VOLT_SHIFT;
    }
    else if(Dcdc_TmpLong <= (long)DCDC_PWMMIN<<VOLT_SHIFT)
    {
        Dcdc_TmpLong = (long)DCDC_PWMMIN<<VOLT_SHIFT;	
    }

    Dcdcisr.ui_Dcdc_Volt_Out0 = (unsigned int)(Dcdc_TmpLong >> VOLT_SHIFT);
              
   //////////////////////////////////////////////////////-------///// 
    Dcdcisr.v_ubalance_err_l=Dcdcisr.v_ubalance_err;
#if defined(__SIGNAL_SAMPLE)
    Dcdc_Tmp1 = 0;//(signed int)(AdcMirror.ADCRESULT1)+(signed int)(AdcMirror.ADCRESULT11); 
    Dcdc_Tmp = 0;//(signed int)(AdcMirror.ADCRESULT2)+(signed int)(AdcMirror.ADCRESULT12);
#else
    Dcdc_Tmp1=(signed int)(AdcMirror.ADCRESULT1)+(signed int)(AdcMirror.ADCRESULT11); 
    Dcdc_Tmp= (signed int)(AdcMirror.ADCRESULT2)+(signed int)(AdcMirror.ADCRESULT12);
#endif    
    Dcdcisr.v_ubalance_err=Dcdc_Tmp1-Dcdc_Tmp; 
    if((Dcdcisr.v_ubalance_err>600)||(Dcdcisr.v_ubalance_err<-600))
    {
        if(Dcdcisr.v_ubalance_err>600)
        {
            Dcdcisr.v_ubalance_err=600;
        }
        else
        {
            Dcdcisr.v_ubalance_err=-600;
        }
        if(uiActionReady>1)
        {
            Dcdcisr.unbalance_cont++;
        }
        if(Dcdcisr.unbalance_cont>=100)
        {
            EPwm1Regs.AQCSFRC.all = 0x05;		//PWM1A,PWM1B
            EPwm2Regs.AQCSFRC.all = 0x05;		//PWM2A,PWM2Bÿ
            EPwm3Regs.AQCSFRC.all = 0x05;		//PWM3A,PWM3B
            EPwm4Regs.AQCSFRC.all = 0x05;		//PWM4A,PWM4Bÿ
            uiActionReady=0;
            Dcdcisr.ui_poweron_off=0;           //
            Dcdcisr.unbalance_cont=0;
            Dcdcisr.test=100;
        }
    }
    else
    {  
        Dcdcisr.unbalance_cont=0;
    }
#ifdef __DEBUG
    Dcdcisr.unbalance_cont = 0;
    Dcdcisr.v_ubalance_err = 0;
#endif
    Dcdcisr.v_pwm_adj=Dcdcisr.v_pwm_adj+((Dcdcisr.v_ubalance_err-Dcdcisr.v_ubalance_err_l)>>1)+(Dcdcisr.v_ubalance_err>>4);
    if(Dcdcisr.v_pwm_adj>=1588)
    {
        Dcdcisr.v_pwm_adj=1588;
    }
    else if(Dcdcisr.v_pwm_adj<=-1588)
    {
        Dcdcisr.v_pwm_adj=-1588;
    }

    AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;                   //  	
    Dcdcisr.ui_Dcdc_Pwm_Out = Dcdcisr.ui_Dcdc_Volt_Out0;
    
    if(Dcdcisr.ui_Dcdc_Pwm_Out<=dPERIOD_OUT_MIN)        //»·Â·¼ÆËãÆµÂÊ>350KHz
    { 	
        Dcdc_Tmp1=dPERIOD_OUT_MIN;                      //°´ÕÕ350KHz¼ÆËã
    }
    else
    {
        Dcdc_Tmp1=Dcdcisr.ui_Dcdc_Pwm_Out;              //°´ÕÕÊµ¼ÊÆµÂÊ¼ÆËã     
    }
	Dcdc_TmpLong=(long)Dcdc_Tmp1*Dcdcisr.v_pwm_adj; 
    Dcdcisr.v_adj_dead=Dcdc_TmpLong>>21;
	
    if(Dcdcisr.v_adj_dead>=0)
    {
        Dcdcisr.v_adj_dead12=(unsigned int)(Dcdcisr.v_adj_dead);
        Dcdcisr.v_adj_dead34=0;
    }
    else
    {
        Dcdcisr.v_adj_dead12=0;
        Dcdcisr.v_adj_dead=-Dcdcisr.v_adj_dead;
        Dcdcisr.v_adj_dead34=(unsigned int)(Dcdcisr.v_adj_dead);
    }
//2019-02-15
    //Çý¶¯ÆµÂÊ½ÏµÍ£¬»òµçÁ÷½Ï´ó£¬»òÊä³öµçÑ¹µÍÊ±£¬ÄÚ¹Ü±¾¾Í¹¤×÷ÔÚÈí¿ª¹ØÄ£Ê½£¬²»ÐèÒªÔÙÔö´óÄÚ¹ÜËÀÇø
    if((Dcdcisr.ui_Dcdc_Pwm_Out > dFREQ_TO_LOUT(310))   ||\
        (iq10MdlCurr.lData > 3536l)                     ||\
        (iq10MdlVolt.lData <= 368640l)                  ||\
        ((iq10MdlCurr.lData > 1536l) && (iq10MdlVolt.lData <= 491520l)))
    {
        Dcdcisr.ui_dead_adj_min = dDEAD_ADJ_MIN_NF;             //Õý³£µÄÄÚ¹ÜËÀÇø
    }
    else if(Dcdcisr.ui_Dcdc_Pwm_Out < dFREQ_TO_LOUT(320))       //¸ßÆµ>320.5K
    {
        Dcdcisr.ui_dead_adj_min = dDEAD_ADJ_MIN_HF;             //½Ï´óÄÚ¹ÜËÀÇø
    }
//2018-02-15 end
    /////////////////////////////////////////////////////--------////   
    if(uiActionReady>1)
    {
        if(Dcdcisr.ui_Dcdc_Pwm_Out > dPERIOD_OUT_MIN)                                //»·Â·µ÷ÕûÆµÂÊ<350KHzÊ±
        {
            if(Dcdcisr.ui_Dcdc_Pwm_Out >= dRESONANCE_POINT)
            {
                Dcdcisr.ui_Dcdc_Dead_adj=0;
                Dcdcisr.ui_Dcdc_Dead_adj1_temp=Dcdcisr.voref_dead_adj;
            }
            else
            {
                Dcdc_Tmp = dRESONANCE_POINT - Dcdcisr.ui_Dcdc_Pwm_Out;              //dRESONANCE_POINT==21333 1/(21333/64*2)==150kHz , Ó²¼þ²ÎÊýÐ³ÕñµãÔ¼Îª150kHz     21333-32640==-11307 2017 20k//20170522 tempup
                Dcdcisr.ui_Dcdc_Dead_adj = (unsigned int)(Dcdc_Tmp >> 10);
                if(Dcdc_Tmp>=8192)
                {
                    Dcdcisr.ui_Dcdc_Dead_adj1_temp=dDEAD_ADJ1_MAX;                  //20170522 tempup    
                }
                else
                {
                    Dcdcisr.ui_Dcdc_Dead_adj1_temp=(unsigned int)(((long)Dcdc_Tmp*dDEAD_ADJ1_MAX)>>13);//20170522 tempup
                }

                if(Dcdcisr.ui_Dcdc_Dead_adj1_temp<=Dcdcisr.voref_dead_adj)
                {
                    Dcdcisr.ui_Dcdc_Dead_adj1_temp=Dcdcisr.voref_dead_adj;
                }		
            }
            //if((iq10MdlCurr.lData>=12288)||((uiSelfChkEn)&&(iq10MdlAvgCurr.lData>=5120)))
            if((iq10MdlCurr.lData>=1895)||((uiSelfChkEn)&&(iq10MdlAvgCurr.lData>=2000)))
            {
                Dcdcisr.ui_dead_comp_flag=1;
            }
            //else if(iq10MdlCurr.lData<=10240)
            else if(iq10MdlCurr.lData<=1690)
            {
                Dcdcisr.ui_dead_comp_flag=0;
            }
            if(Dcdcisr.ui_Dcdc_Volt_Ref_ctrol<=il350ref)
            {	
                Dcdcisr.ui_dead_comp_flag2=0;
            }
            else if((Dcdcisr.ui_Dcdc_Volt_Ref_ctrol<il360ref)&&(Dcdcisr.ui_Dcdc_Volt_Ref_ctrol>il350ref))
            {	
                if(Dcdcisr.ui_dead_comp_flag2==0)
                {
                    Dcdcisr.ui_dead_comp_flag2=0;
                }
                else
                {
                    Dcdcisr.ui_dead_comp_flag2=1;
                }
            }
            else if((Dcdcisr.ui_Dcdc_Volt_Ref_ctrol<=il420ref)&&(Dcdcisr.ui_Dcdc_Volt_Ref_ctrol>=il360ref))
            {	
                Dcdcisr.ui_dead_comp_flag2=1;
            }
            else if((Dcdcisr.ui_Dcdc_Volt_Ref_ctrol<il430ref)&&(Dcdcisr.ui_Dcdc_Volt_Ref_ctrol>il420ref))
            {	
                if(Dcdcisr.ui_dead_comp_flag2==1)
                {
                    Dcdcisr.ui_dead_comp_flag2=1;
                }
                else
                {
                    Dcdcisr.ui_dead_comp_flag2=2;
                }
            }
            else if((Dcdcisr.ui_Dcdc_Volt_Ref_ctrol<=il550ref)&&(Dcdcisr.ui_Dcdc_Volt_Ref_ctrol>=il430ref))
            {	
                Dcdcisr.ui_dead_comp_flag2=2;
            }
            else
            {
                Dcdcisr.ui_dead_comp_flag2=3;
            }
            //´Ë´¦ÓÃÓÚ¿ØÖÆÕ¼¿Õ±È²¹³¥(Ôö´óÕ¼¿Õ±È£¬ÊýÖµÔ½´ó£¬Õ¼¿Õ±ÈÔ½Ð¡)
            //ÓÃÓÚÔÚ²»Í¬Êä³öµçÑ¹ÏÂ£¬²¹³¥Õ¼¿Õ±È£¬Ïû³ýÓ²¹Ø¶Ï
            if(Dcdcisr.ui_dead_comp_flag==1)
            {
                if((Dcdcisr.ui_Dcdc_Dead_adj1_temp>=307)&&(Dcdcisr.ui_dead_comp_flag2==0))
                {
                    Dcdcisr.ui_Dcdc_Dead_adj1_temp=307;//20170522 tempup
                }
                if((Dcdcisr.ui_Dcdc_Dead_adj1_temp>=205)&&(Dcdcisr.ui_dead_comp_flag2==1))
                {
                    Dcdcisr.ui_Dcdc_Dead_adj1_temp=205;//20170522 tempup
                }
                if((Dcdcisr.ui_Dcdc_Dead_adj1_temp>=103)&&(Dcdcisr.ui_dead_comp_flag2==2))
                {
                    Dcdcisr.ui_Dcdc_Dead_adj1_temp=103;//20170522 tempup
                }
            }

            Dcdc_uTmpLong= (unsigned long)Dcdcisr.ui_Dcdc_Dead_adj1_temp*Dcdcisr.ui_Dcdc_Pwm_Out;
            Dcdcisr.ui_Dcdc_Dead_adj1=(unsigned int)(Dcdc_uTmpLong>>16);

            EPwm1Regs.AQCSFRC.all = 0x0;		
            EPwm2Regs.AQCSFRC.all = 0x0;
            EPwm3Regs.AQCSFRC.all = 0x0;
            EPwm4Regs.AQCSFRC.all = 0x0;
        }  
        else                                                //½øÈë¼äÐªÄ£Ê½£¬·âËøPWMÊä³ö
        {
            Dcdcisr.ui_Dcdc_Pwm_Out = dPERIOD_OUT_MIN;
            Dcdcisr.ui_Dcdc_Dead_adj = Dcdcisr.ui_dead_adj_min;         //13;                                                
            Dcdcisr.ui_Dcdc_Dead_adj1 = ((dPERIOD_OUT_MIN * dDEAD_ADJ1_MAX) + 32768) >> 16;//57;//20170522 tempup (Dcdcisr.ui_Dcdc_Pwm_Out * 410 / 65536)  
            EPwm1Regs.AQCSFRC.all = 0x05;
            EPwm2Regs.AQCSFRC.all = 0x05;
            EPwm3Regs.AQCSFRC.all = 0x05;
            EPwm4Regs.AQCSFRC.all = 0x05;  
        }
    }
    /**********************************************************/  
    if(uiActionReady>1)   
    {
        if(Dcdcisr.ui_Dcdc_Dead_adj1<=(Dcdcisr.ui_Dcdc_Dead_adj+24))
        {
            Dcdcisr.ui_Dcdc_Dead_adj1=Dcdcisr.ui_Dcdc_Dead_adj+24;
        }
        Dcdcisr.PERIOD_SHADOW=Dcdcisr.ui_Dcdc_Pwm_Out >> 6;

        Dcdcisr.PWM1A_SHADOW=(Dcdcisr.ui_Dcdc_Pwm_Out >> 7)+Dcdcisr.ui_Dcdc_Dead_adj1+Dcdcisr.v_adj_dead12;
        Dcdcisr.PWM2A_SHADOW=(Dcdcisr.ui_Dcdc_Pwm_Out >> 7)-Dcdcisr.ui_Dcdc_Dead_adj1-Dcdcisr.v_adj_dead12;

        Dcdcisr.PWM1B_SHADOW=(Dcdcisr.ui_Dcdc_Pwm_Out >> 7)+Dcdcisr.ui_dead_adj_min+Dcdcisr.ui_Dcdc_Dead_adj;
        Dcdcisr.PWM2B_SHADOW=(Dcdcisr.ui_Dcdc_Pwm_Out >> 7)-Dcdcisr.ui_dead_adj_min-Dcdcisr.ui_Dcdc_Dead_adj;

        Dcdcisr.PWM3A_SHADOW=Dcdcisr.ui_dead_adj_min+Dcdcisr.ui_Dcdc_Dead_adj1+Dcdcisr.ui_Dcdc_Dead_adj+Dcdcisr.v_adj_dead34;
        if(Dcdcisr.PWM3A_SHADOW>=Dcdcisr.PERIOD_SHADOW-1)
        {
            Dcdcisr.PWM3A_SHADOW=Dcdcisr.PERIOD_SHADOW-1;
        }
        Dcdcisr.PWM3B_SHADOW=(Dcdcisr.ui_dead_adj_min * 2)+(Dcdcisr.ui_Dcdc_Dead_adj<<1);
        Dcdcisr.PWM4A_SHADOW=(Dcdcisr.ui_Dcdc_Pwm_Out >> 6)-Dcdcisr.ui_dead_adj_min-Dcdcisr.ui_Dcdc_Dead_adj1-Dcdcisr.ui_Dcdc_Dead_adj-Dcdcisr.v_adj_dead34;
        if(Dcdcisr.PWM4A_SHADOW<=1)
        {
            Dcdcisr.PWM4A_SHADOW=1;
        }
        Dcdcisr.PWM4B_SHADOW=(Dcdcisr.ui_Dcdc_Pwm_Out >> 6)-(Dcdcisr.ui_dead_adj_min * 2)-(Dcdcisr.ui_Dcdc_Dead_adj<<1);
    }
    else
    {
        EPwm1Regs.AQCSFRC.all = 0x05;		//PWM1A,PWM1B force low
        EPwm2Regs.AQCSFRC.all = 0x05;		//PWM2A,PWM2B force lowÿ
        EPwm3Regs.AQCSFRC.all = 0x05;		//PWM3A,PWM3B force low
        EPwm4Regs.AQCSFRC.all = 0x05;		//PWM4A,PWM4B force low
        
        Dcdcisr.PERIOD_SHADOW = 166; 
        Dcdcisr.PWM1A_SHADOW = 113;	  	
        Dcdcisr.PWM1B_SHADOW = 103;

        Dcdcisr.PWM2A_SHADOW = 53;	
        Dcdcisr.PWM2B_SHADOW = 63;

        Dcdcisr.PWM3A_SHADOW = 50;
        Dcdcisr.PWM3B_SHADOW = 40;

        Dcdcisr.PWM4A_SHADOW = 116;
        Dcdcisr.PWM4B_SHADOW = 126;
    }
/**********************************************************/	
//*********************************************************************//       
    if((EPwm1Regs.TBSTS.bit.CTRDIR==1)||((EPwm1Regs.TBSTS.bit.CTRDIR==0)&&(EPwm1Regs.TBCTR>129)))
    {    
        EPwm1Regs.TBPRD=Dcdcisr.PERIOD_SHADOW;
        EPwm2Regs.TBPRD=EPwm3Regs.TBPRD=EPwm4Regs.TBPRD=EPwm1Regs.TBPRD;

        EPwm4Regs.CMPA.half.CMPA =Dcdcisr.PWM4A_SHADOW;
        EPwm4Regs.CMPB =Dcdcisr.PWM4B_SHADOW;

        EPwm2Regs.CMPA.half.CMPA=Dcdcisr.PWM2A_SHADOW;
        EPwm2Regs.CMPB=Dcdcisr.PWM2B_SHADOW;

        EPwm1Regs.CMPA.half.CMPA=Dcdcisr.PWM1A_SHADOW;
        EPwm1Regs.CMPB=Dcdcisr.PWM1B_SHADOW;

        EPwm3Regs.CMPA.half.CMPA =Dcdcisr.PWM3A_SHADOW; 
        EPwm3Regs.CMPB =Dcdcisr.PWM3B_SHADOW;       

        Dcdcisr.PWM_load_flag = 0x00;
    }
    else
    {
        Dcdcisr.PWM_load_flag = 0x01;
    }
//*********************************************************************// 
     
    AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 1;          //

    Dcdcisr.ui_Dcdc_Curr_Cnt++;
    Dcdcisr.ul_Dcdc_Curr_Sum = Dcdcisr.ul_Dcdc_Curr_Sum + (unsigned long)Dcdcisr.ui_Dcdc_Curr_Adc;
    Dcdcisr.ul_Dcdc_Volt_Sum = Dcdcisr.ul_Dcdc_Volt_Sum + (unsigned long)Dcdcisr.ui_Dcdc_Volt_Adc;

	if (Dcdcisr.ui_Dcdc_Curr_Cnt >= 64)
	{
		Dcdcisr.ui_Dcdc_Curr_Cnt = 0;
		Dcdcisr.ui_Dcdc_Curr_Ave0 = (unsigned int)(Dcdcisr.ul_Dcdc_Curr_Sum>>7);
		Dcdc_uTmpLong = (unsigned long)Dcdcisr.ui_Dcdc_Curr_Filt_K1*Dcdcisr.ui_Dcdc_Curr_Dis0+(((unsigned long)Dcdcisr.ui_Dcdc_Curr_Filt_K1*Dcdcisr.ui_Dcdc_Curr_Dis0_Lo)>>15)
			+(unsigned long)Dcdcisr.ui_Dcdc_Curr_Filt_K2*Dcdcisr.ui_Dcdc_Curr_Ave0+(unsigned long)Dcdcisr.ui_Dcdc_Curr_Filt_K2*Dcdcisr.ui_Dcdc_Curr_Ave1;
			
		Dcdcisr.ul_Dcdc_Curr_Sum = 0;
		Dcdcisr.ui_Dcdc_Curr_Dis0 = (unsigned int)(Dcdc_uTmpLong >> 15);
		Dcdcisr.ui_Dcdc_Curr_Dis0_Lo = (unsigned int)(Dcdc_uTmpLong & 0x7FFF);
		
		Dcdcisr.ui_Dcdc_Curr_Ave1 = Dcdcisr.ui_Dcdc_Curr_Ave0;	
		Dcdcisr.ul_Dcdc_Volt_Ave = Dcdcisr.ul_Dcdc_Volt_Sum>>4;
		if(Dcdcisr.ul_Dcdc_Volt_Ave<500)
		{
			Dcdcisr.ul_Dcdc_Volt_Ave = 500;
		}	
		Dcdcisr.ul_Dcdc_Volt_Sum = 0;
	}

	if (Dcdcisr.ui_Dcdc_Curr_Max<Dcdcisr.ui_Dcdc_Curr_Adc)
	{
		Dcdcisr.ui_Dcdc_Curr_Max = Dcdcisr.ui_Dcdc_Curr_Adc;
	}

    //¶ÌÂ·½â³ý
	if ((Dcdcisr.ui_Dcdc_Volt_Adc>=1000)&&(Dcdcisr.ui_Dcdc_Short_Flag == 1))
	{
		Dcdcisr.ui_Dcdc_Short_Flag = 0;
		uiMdlStatus &= ~dSHORT_CIRCUIT;
	}
    ////////////////////////////////////////////////////////////////////////////////
    while(Dcdcisr.PWM_load_flag==0x01)
    {
        if(EPwm1Regs.TBSTS.bit.CTRDIR==1)
        {  
            EPwm1Regs.TBPRD=Dcdcisr.PERIOD_SHADOW;
            EPwm2Regs.TBPRD=EPwm3Regs.TBPRD=EPwm4Regs.TBPRD=EPwm1Regs.TBPRD;

            EPwm4Regs.CMPA.half.CMPA =Dcdcisr.PWM4A_SHADOW;
            EPwm4Regs.CMPB =Dcdcisr.PWM4B_SHADOW;

            EPwm2Regs.CMPA.half.CMPA=Dcdcisr.PWM2A_SHADOW;
            EPwm2Regs.CMPB=Dcdcisr.PWM2B_SHADOW;

            EPwm1Regs.CMPA.half.CMPA=Dcdcisr.PWM1A_SHADOW;
            EPwm1Regs.CMPB=Dcdcisr.PWM1B_SHADOW;

            EPwm3Regs.CMPA.half.CMPA =Dcdcisr.PWM3A_SHADOW; 
            EPwm3Regs.CMPB =Dcdcisr.PWM3B_SHADOW;           

            Dcdcisr.PWM_load_flag = 0x00;
        }
    }
    CpuTimer1Regs.TCR.bit.TIF=1;
}

//===========================================================================
// No more.
//===========================================================================

