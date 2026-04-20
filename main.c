/*******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Empty Application Example
*              for ModusToolbox.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2024-2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/


/*******************************************************************************
* Header Files
*******************************************************************************/
#include "mtb_hal.h"
#include "cybsp.h"
#include "mtb_mcdi.h"
#include "svpwm.h"

/*******************************************************************************
* Macros
*******************************************************************************/


/*******************************************************************************
* Global Variables
*******************************************************************************/
uint32_t isr_counter = 0UL; /* Counter for ISR calls */
/*Übung2*/
uint32_t gVar_Poti = 0UL; /* ADC result for Speed potentiometer */
uint32_t gVar_T_Motor = 0UL; /* ADC result for Motor temperature */
uint32_t gVar_V_Bus = 0UL; /* ADC result for VBUS voltage */

uint16_t amplitude = 2200u;   /* Applied amplitude, Max amplitude  is 16384 (Q14)
                                          proportional to applied voltage*/
uint32_t angle     = 0u;      /* Open loop angle*/
uint32_t inc_angle = 10000;  /* Incremental angle in every PWM cycle */

/*SVPWM handler*/
SVPWM_t SVPWM_0;

// Übung 3
bool drive_enable = true;

//Übung 4
uint32_t raw_idc_current_first = 0u;
uint32_t raw_idc_current_second = 0u; 


/*******************************************************************************
* Function Prototypes
*******************************************************************************/


/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CPU. It...
*    1.
*    2.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    
    Cy_CORDIC_Enable(MXCORDIC); /* Enable CORDIC peripheral */

    SVPWM_Init(&SVPWM_0);/*SVM parameter Init*/

    //**** Übung 1 ********/
    /* Assume the HPPASS (as well as other shared resource) is already initialized */
    myMotor_init(); /* Initialize all the motor-specific PWMs and Timers */
    /* Initialize all the rest of solutions/peripherals */
    myMotor_enable(); /* Enable all the motor-specific PWMs and Timers, after this action they are sensitive to input triggers */
    /* Enable all the rest of solutions/peripherals */
    myMotor_start(); /* Safely start the shared resources (if not started yet) and all the motor-specific PWMs and Timers synchronously (except the Slow Timer - it is started asynchronously) */



    /* Enable global interrupts */
    __enable_irq();

    for (;;)
    {
        //Application Code here
    Cy_GPIO_Write(CYBSP_EN_IPM_PORT, CYBSP_EN_IPM_PIN, drive_enable);

    }
}

void vres_0_motor_0_fast_callback(void)
{
    /*Generated the open loop angle for SVPWM module*/
    if (angle > 0xffffff)
    {
        angle = 0;
    }
    else
    {
        angle = angle + gVar_Poti*15;
        if(angle > 0xffffff)
        {
            angle = angle - 0xffffff;
        }
    }

   SVPWM_SVMUpdate(&SVPWM_0, amplitude, angle);
    /* Update the PWM compare values */
    myMotor_mod_U_set((uint16_t)(SVPWM_0.phaseu_crs),(uint16_t)(SVPWM_0.phaseu_crs));
    myMotor_mod_V_set((uint16_t)(SVPWM_0.phasev_crs),(uint16_t)(SVPWM_0.phasev_crs));
    myMotor_mod_W_set((uint16_t)(SVPWM_0.phasew_crs),(uint16_t)(SVPWM_0.phasew_crs));
    myMotor_mod_update();

    raw_idc_current_first = myMotor_IDC_get_first_result();
    raw_idc_current_second = myMotor_IDC_get_second_result();

}
void vres_0_motor_0_slow_callback(void)
{
    /* Slow callback implementation */
        
    Cy_GPIO_Inv(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN);

    gVar_V_Bus = myMotor_VBUS_get_result();
    gVar_Poti = myMotor_SPEED_AN_get_result();
    gVar_T_Motor = myMotor_T_POWER_get_result();

}

/* [] END OF FILE */
