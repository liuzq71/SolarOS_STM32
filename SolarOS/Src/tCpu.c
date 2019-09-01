/*
*********************************************************************************************************
*
*	ģ������   :   �����л�ʵ��ģ�顣
*	�ļ�����   :   tCpu.c
*	��    ��   :   V1.0
*	˵    ��   :   ��Ҫ��PendSV_Handler()��ʵ�֣��ȱ��浱ǰ����ļĴ���
*                ����Ӧ�Ķ�ջ�У��ٴ���һ����Ҫ���е�����Ķ�ջ�лָ�
*                ��Ӧ�ļĴ������Ա�������һ����
*
*	�޸ļ�¼   :
*		             �汾��     ����         ����            ˵��
*		              V1.0    2019-4-16      Solar           ��һ�ΰ汾����
*		              
*
*	��Ȩ       ��  ����ѧϰ����ʹ�ã���ֹ������ҵ��;
*
*********************************************************************************************************
*/

#include "SOLAROS.h"
#include <ARMCM3.h>

#define NVIC_INT_CTRL       0xE000ED04         /*�ж�ʹ�ܿ��ƼĴ���*/ 
#define NVIC_PENDSVSET      0x10000000         /*��λPENDSV�ж�*/ 
#define NVIC_SYSPRI2        0xE000ED22         /*�ж����ȼ����ƼĴ���*/  
#define NVIC_PENDSV_PRI     0x000000FF         /*����PENDSV�жϵ����ȼ�Ϊ���*/  

#define MEM32(addr)         *(volatile unsigned long *)(addr)
#define MEM8(addr)          *(volatile unsigned char *)(addr)
	
/*
*********************************************************************************************************
*	�� �� ��  :   tTaskRunFirst
*	����˵��  :   ����PSP�Ĵ���Ϊ0������ǵ�һ��ִ�������л�������PENDSV�ж����ȼ�Ϊ��Ͳ��������ж�
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskRunFirst(void)
{

    __set_PSP(0);                             /*����PSP�Ĵ���Ϊ0������ǵ�һ��ִ�������л�*/

    MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;     /*����PENDSV�жϵ����ȼ�Ϊ���*/
    
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;    /*����PENDSV�жϣ���PENDSV�жϷ��������ִ�������л�*/

}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskSwitch
*	����˵��  :   ����PENDSV�ж�
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskSwitch(void)
{
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;       /*����PENDSV�жϣ���PENDSV�жϷ��������ִ�������л�*/
}

/*
*********************************************************************************************************
*	�� �� ��  :   PendSV_Handler
*	����˵��  :   ���жϷ��������ִ�������л������浱ǰ�����ֳ����ָ���һ������ֳ�
*	��    ��  :   ��
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
__asm void PendSV_Handler(void)
{   
    IMPORT  currentTask                        /*����ȫ�ֱ���currentTask*/ 
    IMPORT  nextTask                           /*����ȫ�ֱ���nextTask*/ 
    
    MRS     R0, PSP                            /*��ȡ��ǰ����Ķ�ջָ���ֵ*/ 
    CBZ     R0, PendSVHandler_nosave           /*������tTaskRunFirst��������ִ�����汣��Ĵ����Ĳ�����ֱ��ִ��PendSVHandler_nosave���ָ��Ĵ���*/ 
                                               
    STMDB   R0!, {R4-R11}                      /*��R4-R11�⣬�����Ĵ������Զ����棬������Ҫ�ֶ�����R4-R11*/     
                                             
    LDR     R1, =currentTask                   /*����ջ��λ��currentTask����Ϊ��tTask���ݽṹ�У�stackλ����ʼλ�ã����Ա���currentTask*/
    LDR     R1, [R1]                           /*Ҳ���Ǳ����˶�ջ��ջ��*/
    STR     R0, [R1]                           

PendSVHandler_nosave                           /*�ָ���һ����ļĴ���*/
                                       
    LDR     R0, =currentTask                   
    LDR     R1, =nextTask                      
    LDR     R2, [R1]                           
    STR     R2, [R0]                           /*��currentTask����ΪnextTask����һ�����Ϊ��ǰ����*/
 
    LDR     R0, [R2]                           /*��currentTask�Ķ�ջ�п�ʼ�ָ��Ĵ���*/
    LDMIA   R0!, {R4-R11}                      /*�ָ�R4-R11�Ĵ���*/

    MSR     PSP, R0                            /*�ָ���ջָ�뵽PSP*/
    ORR     LR, LR, #0x04                      /*ʹ��PSP��ָ��Ķ�ջΪ��ǰ�Ķ�ջ��PendSVʹ�õ���MSP*/
    BX      LR                                 /*�˳�PendSV�쳣����*/
}  

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskEnterCritical
*	����˵��  :   �����ٽ���
*	��    ��  :   ��
*	�� �� ֵ  :   �����ٽ���ǰ�����ȼ�����Ĵ�����ֵ
*********************************************************************************************************
*/
uint32_t tTaskEnterCritical(void)
{
	uint32_t status = __get_PRIMASK();
	__disable_irq();
	return status;
}

/*
*********************************************************************************************************
*	�� �� ��  :   tTaskExitCritical
*	����˵��  :   �˳��ٽ���
*	��    ��  :   status�������ٽ���ǰ�����ȼ�����Ĵ�����ֵ
*	�� �� ֵ  :   ��
*********************************************************************************************************
*/
void tTaskExitCritical(uint32_t status)
{
	__set_PRIMASK(status);
}

/***************************** SOLAROS (END OF FILE) *********************************/