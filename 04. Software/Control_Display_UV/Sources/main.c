/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "Vtpm3ovf.h"
/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

/* User includes (#include below this line is not maintained by Processor Expert) */







// Definicion de las estados de los pines generales
#define		PIN_COMO_SALIDA									1
#define		PIN_COMO_ENTRADA								0

// Definiciones para habilitar o no el pull up interno que tienen los pines cuando se usan como entradas digitales
#define		PULL_UP_HABILITADO								1
#define		PULL_UP_DESHABILITADO							0



// El Backlight del LCD se debe encender mediante un pin especifico, por disenio de la placa del manillar de vacio
#define 	BACLKIGHT_LCD_DIRECCION							PTGDD_PTGDD1
#define 	BACLKIGHT_LCD_PIN								PTGD_PTGD1






	////// **************************************************************************************** //////
	////// 									MANEJO DEL LCD											//////
	////// **************************************************************************************** //////

// Instrucciones para borrar la pantalla
#define 	INSTRUCCION_LCD_BORRAR_PANTALLA		0x01

// Instrucciones para regresar el cursor al inicio
#define 	INSTRUCCION_LCD_REGRESAR_CURSOR		0x20

// Instrucciones para seleccionar el modo
#define 	INSTRUCCION_LCD_MODO				0x04
#define 	INSTRUCCION_LCD_MODO_INCREMENTO		0X02
#define 	INSTRUCCION_LCD_MODO_DECREMENTO		0X00
#define 	INSTRUCCION_LCD_MODO_SHIFT_ON		0X01
#define 	INSTRUCCION_LCD_MODO_SHIFT_OFF		0X00

// Instrucciones para encender o apagar la pantalla
#define 	INSTRUCCION_LCD_PANTALLA			0x08
#define 	INSTRUCCION_LCD_PANTALLA_ON			0x04
#define 	INSTRUCCION_LCD_PANTALLA_OFF		0x00
#define 	INSTRUCCION_LCD_PANTALLA_CURSOR_ON	0x02
#define 	INSTRUCCION_LCD_PANTALLA_CURSOR_OFF	0x00
#define 	INSTRUCCION_LCD_PANTALLA_BLINK_ON	0x01
#define 	INSTRUCCION_LCD_PANTALLA_BLINK_OFF	0x00

// Instrucciones para desplazar el cursor o la pantalla
#define 	INSTRUCCION_LCD_DESPLAZAR			0x10
#define 	INSTRUCCION_LCD_DESPLAZAR_PANTALLA	0x08
#define 	INSTRUCCION_LCD_DESPLAZAR_CURSOR	0x00
#define 	INSTRUCCION_LCD_DESPLAZAR_DERECHA	0x04
#define 	INSTRUCCION_LCD_DESPLAZAR_IZQUIERDA	0x00

// Instrucciones para definir la interfaz
#define 	INSTRUCCION_LCD_INTERFAZ			0x20
#define 	INSTRUCCION_LCD_INTERFAZ_8BITS		0x10
#define 	INSTRUCCION_LCD_INTERFAZ_4BITS		0x00
#define 	INSTRUCCION_LCD_INTERFAZ_2LINEAS	0x08
#define 	INSTRUCCION_LCD_INTERFAZ_1LINEA		0x00
#define 	INSTRUCCION_LCD_INTERFAZ_5X10		0x04
#define 	INSTRUCCION_LCD_INTERFAZ_5X8		0x00

// Instrucciones para definir el renglon sobre el que se escribe
#define 	INSTRUCCION_LCD_RENGLON_SUPERIOR		0x80
#define 	INSTRUCCION_LCD_RENGLON_INFERIOR		INSTRUCCION_LCD_RENGLON_SUPERIOR + 0x40



	/* Defines para Display MangoVacio */

// Data direction - Para elegir si son entradas o salidas
#define  	LCD_RS_DD			PTEDD_PTEDD3 //rs
#define 	LCD_E_DD			PTEDD_PTEDD4 //e
#define 	LCD_DB4_DD 			PTEDD_PTEDD5 //db4
#define 	LCD_DB5_DD  		PTEDD_PTEDD6 //db5
#define		LCD_DB6_DD			PTEDD_PTEDD7 //db6
#define 	LCD_DB7_DD			PTGDD_PTGDD0 //db7

// Data - Para escribir o leer los bits de datos
#define  	LCD_RS				PTED_PTED3		//rs
#define 	LCD_E				PTED_PTED4		//e
#define 	LCD_DB4 			PTED_PTED5		//db4
#define 	LCD_DB5	    		PTED_PTED6		//db5
#define		LCD_DB6				PTED_PTED7		//db6
#define 	LCD_PUERTO			PTED			//es el puerto donde van DB6, DB5 y DB4
#define 	LCD_DB7				PTGD_PTGD0		//db7

// Mascaras para acceder a los bits de datos
#define		BORRAR_DBX_PUERTO_MANGO_VACIO		0x1F		// Para dejar 000xxxxx en el PTE que tiene la configuracion DB6-DB5-DB4-xxxxx
#define		MASCARA_DB654_DATO_MANGO_VACIO		0x70		// Para quedarse con 0xxx0000
#define		MASCARA_DB210_DATO_MANGO_VACIO		0x07		// Para quedarse con 00000xxx
#define		MASCARA_DB7_DATO_MANGO_VACIO		0x80		// Para quedarse con x0000000
#define		MASCARA_DB3_DATO_MANGO_VACIO		0x08		// Para quedarse con 0000x000
#define		ROTAR_DB654_MANGO_VACIO				1			// Desaplazar 0xxx0000 a xxx00000
#define		ROTAR_DB210_MANGO_VACIO				5			// Desaplazar 00000xxx a xxx00000




// Las demoras de menos de 1mSeg se hacen bloqueantes
#define 	DEMORA_200US		TPM1SC = 0x00;		TPM1MOD = 0xFFFF;	TPM1CNT = 0;	TPM1SC |= 0x0A;		while( TPM1CNT <= 144 )	{};
#define		ACTIVADO			1
#define		DESACTIVADO			0
#define		PARTE_ALTA			TRUE
#define		PARTE_BAJA			FALSE
#define 	RENGLON_SUPERIOR	1
#define 	RENGLON_INFERIOR	0


	/* FUNCIONES PARA EL LCD */

	// Acceso externo
void InicializarLCD_4bits ( void );
bool EscribirMensajeLCD ( unsigned char renglon, unsigned char inicioTexto, unsigned char totalDeCaracteres, unsigned char *pTexto);

	// Acceso interno
void inicializarPuertoLCD( void );
void demoraInicialLCD( void );
void enviarPrimerInstruccionPara4Bits( char dato );
void enviarInstruccion( char dato );
void enviarDato( char dato );
void mandarNibbleAltoLCD_Vacio( char dato );
void mandarNibbleBajoLCD_Vacio( char dato );



void MostrarParametroEnLCD_TiempoAcumulado ( void );
void MostrarParametroEnLCD_VolumenAcumulado ( void );
void MostrarParametroEnLCD_MensajeInicial ( void );











// Variables para llevar la cuenta del tiempo de funcionamiento del equipo
unsigned char	segundos = 0;
unsigned char	minutos = 0;
unsigned int	horas = 0;
unsigned char	demoraInicial = 0;

// Variable para llevar la cuenta del volumen procesado por el equipo
double			metrosCubiertos = 0;

// Variables para sincronizar el timer
unsigned char	ticksParaElKernel = 0;
bool			flagTerminoElTimerDelKernel = FALSE;







void main ( void )
{
	/* Write your local variable definition here */
	
	
	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/
	/* Write your code here */
	
	
	// Se define como salida al pin para alimentar el backlight del LCD y se lo deja encendido permanentemente
	BACLKIGHT_LCD_DIRECCION = PIN_COMO_SALIDA;
	BACLKIGHT_LCD_PIN = 1;
	
	// Se inicializa el LCD
	InicializarLCD_4bits();
	

	// Se inicializa el timer 3, para generar una interrupcion cada 10ms
	setReg8( TPM3SC, 0x00 );						// Se deshabilita el modulo para configurarlo
	TPM3MOD = 10000;								// Se carga el periodo para la interrupcion del Kernel
	TPM3CNT = 0;									// Se resetea el contador
	setReg8( TPM3SC, 0x48 );						// Se habilita la interrupcion
	
	
	
	// Se imprime un mensaje de inicio
	MostrarParametroEnLCD_MensajeInicial();
	for( demoraInicial = 0; demoraInicial < 5; demoraInicial++ )
	{
		// Se espera en modo stand by hasta que el timer active el flag en la interrupcion
		while( flagTerminoElTimerDelKernel == FALSE )
			asm( wait );
		
		// Se borra el flag
		flagTerminoElTimerDelKernel = FALSE;
	}
	
	
	
	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
	/*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;)
  {
	  
	  // Se muestra el volumen lleve procesado el equipo
	  MostrarParametroEnLCD_VolumenAcumulado();
	  
	  // Se muestra el tiempo acumulado
	  MostrarParametroEnLCD_TiempoAcumulado();


		////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////
		// BLOQUE PARA ESPERAR EN STANDBY HASTA QUE DESBORDE EL TIMER //
		////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////
		
		// Se espera en modo stand by hasta que el timer active el flag en la interrupcion
		while( flagTerminoElTimerDelKernel == FALSE )
			asm( wait );
		
		// Se borra el flag
		flagTerminoElTimerDelKernel = FALSE;
		
		
		// Se incrementan los segundos
		segundos++;
		
		// Se verifica que no sobrepasen del limite
		if( segundos >= 60 )
		{
			// Se resetea el contador
			segundos = 0;
			
			// Se incrementan los minutos
			minutos++;
			
			// Se verifica que no sobrepasen del limite
			if( minutos >= 60 )
			{
				// Se resetea el contador
				minutos = 0;
				
				// Se incrementan las horas
				horas++;
			}
		}
		
		// Cada 6 segundos, se incrementa el volumen procesado 
		if( ! ( segundos % 6 ) )
		{
			// Se incrementa el volumen procesado
			metrosCubiertos += 0.027;
		}
		
  }
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/













/* *** **************************************************************************************** *** */
/* *** 								FUNCIONES PARA EL MANEJO DEL LCD							*** */
/* *** **************************************************************************************** *** */



	/* *** FUNCIONES DE ACCESO GENERAL *** */



/************************************************************/
/* InicializarLCD_4bits										*/
/*  														*/
/*  Inicializa el LCD para usar una interfaz de 4 bits.		*/
/*  Se agrega una demora de 120mSeg luego de cada			*/
/*	instruccion para darle tiempo al LCD a procesarlas.		*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void InicializarLCD_4bits( void )
{
	// Variable auxiliar para hacer mas legibles los codigos
	char instruccion;
	
	// Se inicializa el puerto del LCD
	inicializarPuertoLCD();
	
	// Demora de 200mSeg para esperar que se inicie el LCD
	demoraInicialLCD();
	demoraInicialLCD();
	
	// Se inicializa un bus de 4 bits
	enviarPrimerInstruccionPara4Bits( INSTRUCCION_LCD_INTERFAZ + INSTRUCCION_LCD_INTERFAZ_4BITS );
	
	// Se selecciona un bus de 4 bits, la cantidad de lineas y la fuente del LCD
	instruccion = INSTRUCCION_LCD_INTERFAZ + INSTRUCCION_LCD_INTERFAZ_4BITS;
	instruccion += INSTRUCCION_LCD_INTERFAZ_2LINEAS;
	instruccion += INSTRUCCION_LCD_INTERFAZ_5X8;
	enviarInstruccion( instruccion );
	demoraInicialLCD();
	
	// Se enciende la pantalla. Por default se usa el cursor apagado y sin parpadeo
	enviarInstruccion( INSTRUCCION_LCD_PANTALLA + INSTRUCCION_LCD_PANTALLA_ON + INSTRUCCION_LCD_PANTALLA_CURSOR_OFF + INSTRUCCION_LCD_PANTALLA_BLINK_OFF );
	demoraInicialLCD();
	
	// Se borra la pantalla
	enviarInstruccion( INSTRUCCION_LCD_BORRAR_PANTALLA );
	demoraInicialLCD();
	
	// Se selecciona un modo incremental
	enviarInstruccion( INSTRUCCION_LCD_MODO + INSTRUCCION_LCD_MODO_INCREMENTO + INSTRUCCION_LCD_MODO_SHIFT_OFF );
	demoraInicialLCD();
	
	// Se deja el cursor en la primer posicion
	enviarInstruccion( INSTRUCCION_LCD_RENGLON_SUPERIOR );
	demoraInicialLCD();
}



/****************************************************************/
/* EscribirMensajeLCD											*/
/*  															*/
/*  Imprime en el LCD el texto suministrado, en el renglon 		*/
/*  solicitado y con el offset requerido.						*/
/*  															*/
/*  Recibe: 													*/
/*	 - renglon: Para indicar el numero del renglon, siendo 1 el */
/*  		superior y numerando en orden cresciente los demas  */
/*   - inicioTexto: En que posicion del renglo empieza el texto */
/*   - totalDeCaracteres: Cuantos se van a imprimir del total	*/
/*  		del texto suministrado								*/
/*   - *pTexto: Un puntero al vector donde se almacena el		*/
/*  		mensaje a mostrar									*/
/*  Devuelve: 													*/
/*	 - True: Todos los parametros estaba bien y no hubo			*/
/*			problemas											*/
/*	 - False: Algun parametro estaba mal						*/
/****************************************************************/
bool EscribirMensajeLCD ( unsigned char renglon, unsigned char inicioTexto, unsigned char totalDeCaracteres, unsigned char *pTexto )
{
	// Variables auxiliares
	char caracter = 0;
	char instruccion;
	
	// Se verifica que el parametro no este fuera de rango
	if( totalDeCaracteres == 0 )
		return( FALSE );
	
	// Se manda la orden para posicionarse segun el renglon que se vaya a utilizar
	switch( renglon )
	{
		case RENGLON_SUPERIOR:
			instruccion = INSTRUCCION_LCD_RENGLON_SUPERIOR;
			break;
		case RENGLON_INFERIOR:
			instruccion = INSTRUCCION_LCD_RENGLON_INFERIOR;
			break;
		
		default:
			return( FALSE );
	}
	
	// Se le agrega el inicio del cursor
	if( inicioTexto < 8 )
		instruccion += inicioTexto;
	
	// Se envia la instruccion para posicionar el cursor
	enviarInstruccion( instruccion );
	
	// Se envia el mensaje
	for( caracter = 0; caracter < totalDeCaracteres; caracter++ )
		enviarDato( pTexto[ caracter ] );

}





	/* *** FUNCIONES DE ACCESO INTERNO *** */



/************************************************************/
/* inicializarPuertoLCD										*/
/*  														*/
/*  Define los pines hacia el LCD como salidas y los pone a */
/*  0. Ademas, enciende el backlight para el mango de vacio */
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void inicializarPuertoLCD()
{
	// Se inicializan los puertos como salida
	LCD_RS_DD	= PIN_COMO_SALIDA;
	LCD_E_DD	= PIN_COMO_SALIDA;
	PTEDD		|= 0xE0;
	PTGDD		|= 0x01;
	
	// Se ponen a 0 todas las salidas, por las dudas.
	LCD_RS		= 0;
	LCD_E		= 0;
	LCD_DB4		= 0;
	LCD_DB5 	= 0;
	LCD_DB6		= 0;
	LCD_DB7		= 0;
}



/****************************************************************/
/* demoraInicialLCD												*/
/*  															*/
/*  Demora de 200mSeg para que el LCD procese las ordenes		*/
/*  de inicializacion.											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void demoraInicialLCD ( void )
{
	// Se deja el timer corriendo libre, y se aguarda a que llegue a cierto valor
	TPM1MOD = 0xFFFF;
	
	// Se resetea el contador
	TPM1CNT = 0;
	
	// Se habilita el modulo (con el bus interno) y un prescaler de 128
	TPM1SC = 0x0F;
	
	// Se aguarda hasta llegar a la cuenta necesaria
	while( TPM1CNT <= 0x1000 )	{};
	
	// Se desactiva el modulo y las interrupciones
	TPM1SC = 0x00;
}



	/* ***  	FUNCIONES DE COMUNICACION		  *** */

/****************************************************************/
/* enviarPrimerInstruccionPara4Bits								*/
/*  															*/
/*  Se encarga de mandar el primer dato sobre el bus del puerto.*/
/*  La diferencia con es la demora que se requiere.				*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void enviarPrimerInstruccionPara4Bits( char dato )
{
	// Variables auxiliares
	char auxiliar;
	
	// Primero se deben deshabilitar las lineas de RS y E para poder colocar el dato en el puerto y luego habilitar la linea de E
	LCD_RS = DESACTIVADO;			// Es una instruccion
	LCD_E = DESACTIVADO;			// Para que no tome las modificaciones que se van a realizar sobre el puerto de datos
	DEMORA_200US;
	
	// Se escribe la parte alta de los datos sobre el puerto
	auxiliar = dato & MASCARA_DB654_DATO_MANGO_VACIO;	// Se enmascaran los datos de DB6, DB5 y DB4 
	auxiliar <<= ROTAR_DB654_MANGO_VACIO;				// Se lo desplaza una unidad para que coincidan con los pines PTED7, PTED6 y PTED5
	LCD_PUERTO &= BORRAR_DBX_PUERTO_MANGO_VACIO;		// Se borran los pines del puerto para escribir los datos
	LCD_PUERTO |= auxiliar;								// Se escriben los datos en el puerto. Resta escribir DB7
	auxiliar = dato & MASCARA_DB7_DATO_MANGO_VACIO;
	if( auxiliar > 0 )									// Se escribe el dato DB7
		LCD_DB7 = 1;
	else
		LCD_DB7 = 0;
	
	// Se habilita la linea de E para que el LCD tome la parte del dato
	LCD_E = ACTIVADO;
	
	// Demora de 4ms
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	DEMORA_200US;
	
	LCD_E = DESACTIVADO;			// Se deshabilita la linea de E
}



/****************************************************************/
/* enviarInstruccion											*/
/*  															*/
/*  Se encarga de mandar el dato sobre el bus del puerto.    	*/
/*  Primero se manda la parte alta del dato y luego la baja. 	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void enviarInstruccion( char dato )
{
	// Primero se deben deshabilitar las lineas de RS y E para poder colocar el dato en el puerto y luego habilitar la linea de E
	LCD_RS = DESACTIVADO;			// Es una instruccion
	LCD_E = DESACTIVADO;			// para que no tome las modificaciones que se van a realizar sobre el puerto de datos
	DEMORA_200US;
	
	// Se escribe la parte alta de los datos sobre el puerto
	mandarNibbleAltoLCD_Vacio( dato );
	LCD_E = ACTIVADO;				// Se habilita la linea de E para que el LCD tome la parte del dato
	DEMORA_200US;
	LCD_E = DESACTIVADO;			// Se deshabilita la linea de E
	
	// Se escribe la parte baja de los datos sobre el puerto
	mandarNibbleBajoLCD_Vacio( dato );
	
	LCD_E = ACTIVADO;				// Se habilita la linea de E para que el LCD tome la parte del dato
	DEMORA_200US;
	LCD_E = DESACTIVADO;			// Se deshabilita la linea de E
	
	// Se contempla una demora final, para darle tiempo al LCD a reponerse para los siguientes comandos
	DEMORA_200US;
}



/****************************************************************/
/* enviarDato													*/
/*  															*/
/*  Se encarga de mandar el dato sobre el bus del puerto.    	*/
/*  Primero se manda la parte alta del dato y luego la baja. 	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void enviarDato( char dato )
{
	// Primero se deben deshabilitar las lineas de RS y E para poder colocar el dato en el puerto y luego habilitar la linea de E
	LCD_RS = ACTIVADO;				// Es un dato
	LCD_E = DESACTIVADO;			// para que no tome las modificaciones que se van a realizar sobre el puerto de datos
	DEMORA_200US;
	
	// Se escribe la parte alta de los datos sobre el puerto
	mandarNibbleAltoLCD_Vacio( dato );
	LCD_E = ACTIVADO;				// Se habilita la linea de E para que el LCD tome la parte del dato
	DEMORA_200US;
	LCD_E = DESACTIVADO;			// Se deshabilita la linea de E
	
	// Se escribe la parte baja de los datos sobre el puerto
	mandarNibbleBajoLCD_Vacio( dato );
	LCD_E = ACTIVADO;				// Se habilita la linea de E para que el LCD tome la parte del dato
	DEMORA_200US;
	LCD_E = DESACTIVADO;			// Se deshabilita la linea de E
	
	// Se contempla una demora final, para darle tiempo al LCD a reponerse para los siguientes comandos
	DEMORA_200US;
}





/****************************************************************/
/* mandarNibbleAltoLCD_Vacio									*/
/*  															*/
/*  Envia la parte alta del dato utilizando la configuracion de */
/*  pines que tiene el LCD en el mango de vacio.				*/
/*  															*/
/*  Recibe: El dato a enviar									*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mandarNibbleAltoLCD_Vacio( char dato )
{
	// Variables auxiliares
	char auxiliar;
	
	// Se escribe la parte alta de los datos sobre el puerto
	auxiliar = dato & MASCARA_DB654_DATO_MANGO_VACIO;	// Se enmascaran los datos de DB6, DB5 y DB4 
	auxiliar <<= ROTAR_DB654_MANGO_VACIO;				// Se lo desplaza una unidad para que coincidan con los pines PTED7, PTED6 y PTED5
	LCD_PUERTO &= BORRAR_DBX_PUERTO_MANGO_VACIO;		// Se borran los pines del puerto para escribir los datos
	LCD_PUERTO |= auxiliar;								// Se escriben los datos en el puerto. Resta escribir DB7
	auxiliar = dato & MASCARA_DB7_DATO_MANGO_VACIO;
	if( auxiliar > 0 )									// Se escribe el dato DB7
		LCD_DB7 = 1;
	else
		LCD_DB7 = 0;
	
	DEMORA_200US;
}



/****************************************************************/
/* mandarNibbleBajoLCD_Vacio									*/
/*  															*/
/*  Envia la parte baja del dato utilizando la configuracion de */
/*  pines que tiene el LCD en el mango de vacio.				*/
/*  															*/
/*  Recibe: El dato a enviar									*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mandarNibbleBajoLCD_Vacio( char dato )
{
	// Variables auxiliares
	char auxiliar;
	
	// Se escribe la parte baja de los datos sobre el puerto
	auxiliar = dato & MASCARA_DB210_DATO_MANGO_VACIO;	// Se enmascaran los datos de DB2, DB1 y DB0 
	auxiliar <<= ROTAR_DB210_MANGO_VACIO;				// Se lo desplaza cinco unidades para que coincidan con los pines PTED7, PTED6 y PTED5
	LCD_PUERTO &= BORRAR_DBX_PUERTO_MANGO_VACIO;		// Se borran los pines del puerto para escribir los datos
	LCD_PUERTO |= auxiliar;								// Se escriben los datos en el puerto. Resta escribir DB3
	auxiliar = dato & MASCARA_DB3_DATO_MANGO_VACIO;
	if( auxiliar > 0 )									// Se escribe el dato DB7
		LCD_DB7 = 1;
	else
		LCD_DB7 = 0;
	
	DEMORA_200US;
}
//-------------------------------------------------------------------------------------------------------------------











/************************************************************/
/* MostrarParametroEnLCD_TiempoAcumulado					*/
/*  														*/
/* */
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void MostrarParametroEnLCD_TiempoAcumulado ( void )
{
	// Variables auxiliares
	unsigned int	Copia_Int32;
	unsigned char	renglon[ 8 ];
	unsigned char	digito;
	
	
	// Se escriben los separadores
	renglon[ 2 ] = ':';
	renglon[ 5 ] = ':';

	
	
	// Se copia el valor en la variable local
	Copia_Int32 = segundos;
	
	// Se toman las unidades
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 7 ] = digito;
	Copia_Int32 /= 10;
	
	// Se toman las centenas
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 6 ] = digito;



	// Se copia el valor en la variable local
	Copia_Int32 = minutos;
	
	// Se toman las unidades
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 4 ] = digito;
	Copia_Int32 /= 10;
	
	// Se toman las centenas
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 3 ] = digito;



	// Se copia el valor en la variable local
	Copia_Int32 = horas;
	
	// Se toman las unidades
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 1 ] = digito;
	Copia_Int32 /= 10;
	
	// Se toman las centenas
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 0 ] = digito;
	
	

	// Se imprime el resultado en el LCD
	( void ) EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, renglon );

}



/************************************************************/
/* MostrarParametroEnLCD_VolumenAcumulado					*/
/*  														*/
/* */
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void MostrarParametroEnLCD_VolumenAcumulado ( void )
{
	// Variables auxiliares
	unsigned int	Copia_Int32;
	unsigned char	renglon[ 8 ];
	unsigned char	digito;
	
	
	// Se escriben las unidades
	renglon[ 6 ] = 'm';
	renglon[ 7 ] = '3';

	// Se escribe el separador de decimales
	renglon[ 3 ] = '.';
	
	
	// Se copia el valor en la variable local
	Copia_Int32 = metrosCubiertos * 100;
	
	// Se toman las unidades
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 5 ] = digito;
	Copia_Int32 /= 10;
	
	// Se toman las unidades
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 4 ] = digito;
	Copia_Int32 /= 10;
	
	// Se toman las unidades
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 2 ] = digito;
	Copia_Int32 /= 10;
	
	// Se toman las unidades
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 1 ] = digito;
	Copia_Int32 /= 10;
	
	// Se toman las unidades
	digito = ( Copia_Int32 % 10 ) + '0';
	renglon[ 0 ] = digito;



	// Se imprime el resultado en el LCD
	( void ) EscribirMensajeLCD( RENGLON_INFERIOR, 0, 8, renglon );

}





/************************************************************/
/* MostrarParametroEnLCD_VolumenAcumulado					*/
/*  														*/
/* */
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void MostrarParametroEnLCD_MensajeInicial ( void )
{
	// Variables auxiliares
	unsigned int	Copia_Int32;
	unsigned char	renglon[ 8 ];
	unsigned char	digito;
	
	
	// Se escriben el renglon superior
	renglon[ 0 ] = ' ';
	renglon[ 1 ] = ' ';
	renglon[ 2 ] = 'B';
	renglon[ 3 ] = 'O';
	renglon[ 4 ] = 'D';
	renglon[ 5 ] = 'Y';
	renglon[ 6 ] = ' ';
	renglon[ 7 ] = ' ';


	// Se imprime el resultado en el LCD
	( void ) EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, renglon );
	
	
	
	// Se escriben el renglon superior
	renglon[ 0 ] = ' ';
	renglon[ 1 ] = 'H';
	renglon[ 2 ] = 'E';
	renglon[ 3 ] = 'A';
	renglon[ 4 ] = 'L';
	renglon[ 5 ] = 'T';
	renglon[ 6 ] = 'H';
	renglon[ 7 ] = ' ';


	// Se imprime el resultado en el LCD
	( void ) EscribirMensajeLCD( RENGLON_INFERIOR, 0, 8, renglon );

}
