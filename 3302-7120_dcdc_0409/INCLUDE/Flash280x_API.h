// TI File $Revision: /main/2 $
// Checkin $Date: December 16, 2004   21:15:53 $
//###########################################################################
//
// FILE:  Example_Flash280X_API.h	
//
// TITLE: F280X Flash API example include file
//
// DESCRIPTION:
//
// Function prototypes and macros for the F280x Flash API library examples.
//
//###########################################################################
// $TI Release: F2808, F2806, F2801 API V3.00 $
// $Release Date: August 15, 2005 $
//###########################################################################


#ifndef EXAMPLE_FLASH280X_API_H
#define EXAMPLE_FLASH280X_API_H


/*---- flash program files -------------------------------------------------*/
#include "Flash280x_API_Library.h"

/*---------------------------------------------------------------------------
   Functions used by this example
*---------------------------------------------------------------------------*/

extern void CallFlashAPI(void);        // Kernel function that interfaces to the API
extern void GET_DATA(void);
extern void SEND_DATA(void);
extern void MyCallbackFunction(void);

/*-----------------------------------------------------------------------------
     Specify the PLLCR (PLL Control Register) value.   

      Uncomment the appropriate line by removing the leading double slash: // 
      Only one statement should be uncommented.

      The user's application must set the PLLCR Register before calling any
      of the Flash API functions.
 
      Example:  CLKIN is a 20MHz crystal.  
                The user wants to have a 100Mhz CPU clock (SYSCLKOUT = 100MHz).  
                In this case, PLLCR must be set to 10 (0x000A)
                Uncomment the line: #define PLLCR_VALUE 10
                Comment out the remaining lines with a double slash: //
-----------------------------------------------------------------------------*/

#define PLLCR_VALUE  0x000A     // SYSCLKOUT = (OSCLK*10)/2
// #define PLLCR_VALUE   0x0009     // SYSCLKOUT = (OSCLK*9)/2
// #define PLLCR_VALUE   0x0008     // SYSCLKOUT = (OSCLK*8)/2
// #define PLLCR_VALUE   0x0007     // SYSCLKOUT = (OSCLK*7)/2
// #define PLLCR_VALUE   0x0006     // SYSCLKOUT = (OSCLK*6)/2
// #define PLLCR_VALUE   0x0005     // SYSCLKOUT = (OSCLK*5)/2
// #define PLLCR_VALUE   0x0004     // SYSCLKOUT = (OSCLK*4)/2
// #define PLLCR_VALUE   0x0003     // SYSCLKOUT = (OSCLK*3)/2
// #define PLLCR_VALUE   0x0002     // SYSCLKOUT = (OSCLK*2)/2
// #define PLLCR_VALUE   0x0001     // SYSCLKOUT = (OSCLK*1)/2
// #define PLLCR_VALUE   0x0000     // SYSCLKOUT = (OSCLK)/2 (PLL Bypassed)

#endif // ---- End of EXAMPLE_FLASH280X_API_H     
