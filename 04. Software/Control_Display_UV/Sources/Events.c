#include "Cpu.h"
#include "Events.h"

/* User includes (#include below this line is not maintained by Processor Expert) */


extern	unsigned char	ticksParaElKernel;
extern	bool			flagTerminoElTimerDelKernel;

//-------------------------------------------------------------------------------------------------------------------
ISR( Int_TPM3_Cada20ms )
{
	// Se borra el bit del registro para indicar que se atendio la interrupcion
	clrReg8Bits( TPM3SC, 0x80U ); 
	
	// Se incrementan los ticks para reactivar el Kernel
	ticksParaElKernel--;
	
	// Se verifica si se cumplio el tiempo para reactivar el Kernel
	if( !ticksParaElKernel )
	{
		// Se recarga el valor de ticks necesarios para la proxima reactivacion
		ticksParaElKernel = 100;
		
		// Se activa el flag, para indicar al main que se debe reactivar el Kernel
		flagTerminoElTimerDelKernel = TRUE;
	}
}
//-------------------------------------------------------------------------------------------------------------------


/* END Events */

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.3 [05.09]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
