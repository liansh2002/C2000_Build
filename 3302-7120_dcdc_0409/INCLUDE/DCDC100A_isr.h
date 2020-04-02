/***************************************************************************
*             Defines
***************************************************************************/
#ifndef DCDCisr_H
#define DCDCisr_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
*            Variables Definition
***************************************************************************/
struct dcdc_100a_isr
{

//unsigned int			ui_Dcdc_Mem[1024];			// 
unsigned int            ui_Dcdc_Curr_Out0max;       // 0x9000
unsigned int            ui_Dcdc_Volt_Ref_ctrol;     // 0x9001
unsigned int			ui_Dcdc_Volt_Ref;			// 0x9002
unsigned int			ui_Dcdc_Curr_Ref;			// 0x9003
unsigned int 			ui_Dcdc_Curr_delta;			// 0x9004
unsigned int            PWM_load_flag;              // 0x9005
unsigned int            test;                       // 0x9006
unsigned int            HALF_DUTY;                  // 0x9007
unsigned int            HALF_DUTY_old;              // 0x9008
unsigned int            currlimit_flag;             // 0x9009
unsigned int			ui_Dcdc_Volt_K1;			// 0x900A
unsigned int			ui_Dcdc_Volt_K2;			// 0x900B
unsigned int			ui_Dcdc_Volt_K3;			// 0x900C
unsigned int			ui_Dcdc_Volt_K4;			// 0x900D
unsigned int			ui_Dcdc_Volt_K5;			// 0x900E

unsigned int			ui_Dcdc_Curr_K1;			// 0x900F
unsigned int			ui_Dcdc_Curr_K2;			// 0x9010
unsigned int			ui_Dcdc_Curr_K3;			// 0x9011
unsigned int			ui_Dcdc_Curr_K4;			// 0x9012
unsigned int			ui_Dcdc_Curr_K5;			// 0x9013

unsigned int			ui_Dcdc_Curr_42A;			// 0x9014
unsigned int			ui_Dcdc_Curr_35A;           // 0x9015
unsigned int            ui_Dcdc_Curr_2A;            // 0x9016
unsigned int            ui_Dcdc_Curr_1A;            // 0x9017
unsigned int			ui_Dcdc_Run_Mode;			// 0x9018
unsigned int			ui_Dcdc_Duty_Permit;		// 0x9019
unsigned int			ui_Dcdc_Duty_Ramp;			// 0x901A
unsigned int			ui_Dcdc_Pwm_Out;			// 0x901B
unsigned int 			ui_Dcdc_Dead_adj;			// 0x901C
unsigned int 			ui_Dcdc_Dead_adj1;          // 0x901D
unsigned int 			ui_Dcdc_Dead_adj1_temp;     // 0x901E
unsigned int 			ui_dead_comp_flag;          // 0x901F
unsigned int 			ui_dead_comp_flag2;         // 0x9020
unsigned int            voref_dead_adj;             // 0x9021
unsigned int			ui_Dcdc_Start_Cnt;			// 0x9022

unsigned long			ul_Dcdc_Volt_Sum;			// 0x9024	
unsigned long			ul_Dcdc_Volt_Ave;			// 0x9026
unsigned long			ul_Dcdc_Power_Lim;			// 0x9028
unsigned int			ui_Dcdc_Power_Lim;			// 0x902A

unsigned int			ui_Dcdc_Volt_Adc;			// 0x902B *
signed int				i_Dcdc_Volt_Err0;			// 0x902C
signed int				i_Dcdc_Volt_Err1;			// 0x902D
signed int				i_Dcdc_Volt_Err2;			// 0x902E
unsigned int			ui_Dcdc_Volt_Out0;			// 0x902F
unsigned int			ui_Dcdc_Volt_Out1;			// 0x9030
unsigned int			ui_Dcdc_Volt_Out2;			// 0x9031

unsigned int			ui_Dcdc_Curr_Adc;			// 0x9032
unsigned int			ui_Dcdc_Curr_Adc_load;      // 0x9033
signed int				i_Dcdc_Curr_Err0;			// 0x9034
signed int				i_Dcdc_Curr_Err1;			// 0x9035
signed int				i_Dcdc_Curr_Err2;			// 0x9036
unsigned int			ui_Dcdc_Curr_Out0;			// 0x9037
unsigned int			ui_Dcdc_Curr_Out1;			// 0x9038
unsigned int			ui_Dcdc_Curr_Out2;			// 0x9039
unsigned int			ui_Dcdc_Curr_Max;			// 0x903A


unsigned long 			ul_Dcdc_Curr_Sum;			// 0x903C *
unsigned int 			ui_Dcdc_Curr_Cnt;			// 0x903E *
unsigned int			ui_Dcdc_Curr_Filt_K1;		// 0x903F
unsigned int			ui_Dcdc_Curr_Filt_K2;		// 0x9040
unsigned int 			ui_Dcdc_Curr_Ave0;			// 0x9041
unsigned int 			ui_Dcdc_Curr_Ave1;			// 0x9042
unsigned int			ui_Dcdc_Curr_Dis0;			// 0x9043
unsigned int			ui_Dcdc_Curr_Dis0_Lo;		// 0x9044


unsigned int			ui_Dcdc_Temp_Ctrl;			// 0x9045
unsigned int			ui_Dcdc_Temp_Set1;			// 0x9046
unsigned int			ui_Dcdc_Temp_Set2;			// 0x9047
unsigned int			ui_Dcdc_K_flag;				// 0x9048

unsigned int			ui_Dcdc_Short_Flag;			// 0x9049
unsigned int			ui_Dcdc_Duty_Short;			// 0x904A

unsigned int			ui_Dcdc_debug;				// 0x904B
unsigned int			ui_Dcdc_Vversion;			// 0x904C
unsigned int			ui_Dcdc_Dversion;			// 0x904D
unsigned int			ui_Dcdc_Tversion;			// 0x904E
unsigned int            ui_short_ontime;            // 0x904F
unsigned int            ui_short_permit;            // 0x9050
unsigned int            ui_poweron_off;             // 0x9051
signed  int             i_v_load;                   // 0x9052

signed int              v_ubalance_err;             // 0x9053
signed int              v_ubalance_err_l;           // 0x9054
signed int              v_pwm_adj;                  // 0x9055
signed int              v_adj_dead;                 // 0x9056
unsigned int            v_adj_dead12;               // 0x9057
unsigned int            v_adj_dead34;               // 0x9058
unsigned int            unbalance_cont;             // 0x9059
unsigned int            PERIOD_SHADOW;              // 0x905A
unsigned int            PWM1A_SHADOW;               // 0x905B
unsigned int            PWM1B_SHADOW;               // 0x905C
unsigned int            PWM2A_SHADOW;               // 0x905D
unsigned int            PWM2B_SHADOW;               // 0x905E
unsigned int            PWM3A_SHADOW;               // 0x905F
unsigned int            PWM3B_SHADOW;               // 0x9060
unsigned int            PWM4A_SHADOW;               // 0x9061
unsigned int            PWM4B_SHADOW;               // 0x9062

unsigned int adc_samp0;
unsigned int adc_samp1;
unsigned int ui_dead_adj_min;
};

extern volatile struct dcdc_100a_isr Dcdcisr;


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif

