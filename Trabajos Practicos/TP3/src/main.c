/*
 * main.c
 *
 *  Created on: 5/7/2018
 *      Author: rodrigo
 */

/*============================================================================
 * Autor:   Rodrigo Tirapegui
 * Licencia:
 * Fecha:   22/3/2018
 *===========================================================================*/

/*==================[inlcusiones]============================================*/
#include "app.h"

/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void ){

   // Inicializar los m�dulos que conforman el programa
   if(appInit()) {
      // Correr el superloop del programa
      appRun();
   }

   appAssert();

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

/*==================[fin del archivo]========================================*/



