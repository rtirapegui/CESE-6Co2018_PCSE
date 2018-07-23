/*
 * app.c
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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "app.h"
#include "sapi.h"

/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

DEBUG_PRINT_ENABLE
CONSOLE_PRINT_ENABLE

/* PC UART constantes */
#define UART_PC        UART_USB

/* BLE constantes */
#define UART_BLUETOOTH UART_232

/* IMU constantes */
MPU9250_address_t g_mpu9250Address = MPU9250_ADDRESS_0; // If MPU9250 AD0 pin is connected to GND

/* TASK constantes */
#define TASK_BUFF_SIZE        30
#define TASK_FLOAT_RESOLUTION 6
#define ASSERT_TICK_INTERVAL  500

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[definiciones de funciones internas]=====================*/

/**
 * \brief Formatea un string segun los parametros deseados y lo almacena en buff
 *
 * \param buff puntero al buffer donde almacenar el string formateado
 *
 * \return void Devuelve el numero de bytes escritos en el buffer
 *
 */
uint32_t formatVariadicString(uint8_t *buf, uint32_t bufSize, const uint8_t* format, ...) {

   int32_t size;
   va_list arg;

   va_start(arg, format);
   size = vsnprintf(buf, bufSize, format, arg);
   va_end(arg);

   if(size < 0)
      size = 0;

   return (uint32_t) size;
}

bool_t PCUartInit() {

   // Inicializar UART_USB para conectar a la PC
   debugPrintConfigUart(UART_PC, 9600);

   return TRUE;
}

bool_t IMUInit(void) {

   if(0 <= mpu9250Init(g_mpu9250Address))
      return TRUE;

   return FALSE;
}

bool_t BLEInit(void) {

   // Inicializar UART_232 para conectar al modulo bluetooth
   consolePrintConfigUart(UART_BLUETOOTH, 9600);

   // Testeo del modulo BLE
   uartWriteString(UART_BLUETOOTH, "AT\r\n");
   return waitForReceiveStringOrTimeoutBlocking(UART_BLUETOOTH,
                                                "OK\r\n", strlen("OK\r\n"),
                                                50);

}

void BLEPrintATCommands(void) {
   delay(500);
   uartWriteString(UART_BLUETOOTH, "AT+HELP\r\n" );
}

void BLESendMsg(uint8_t *sTag, uint8_t *sUnits, float data) {

   uint8_t msg[TASK_BUFF_SIZE], sData[TASK_BUFF_SIZE];

   gcvt(data, TASK_FLOAT_RESOLUTION, sData);
   formatVariadicString(msg, sizeof(msg), "%s:%s%s\r\n", sTag, sData, sUnits);

   uartWriteString(UART_BLUETOOTH, msg);
   uartWriteString(UART_PC, msg);
}

/*==================[definiciones de funciones externas]=====================*/
bool_t appInit(void) {

   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma
   boardConfig();

   if(!PCUartInit())
      return FALSE;

   debugPrintlnString("UART_PC configurada.");

   // Inicializar y configurar IMU mpu9250
   if(!IMUInit())
      return FALSE;

   debugPrintlnString("Modulo MPU-09250 conectado correctamente.");

   // Inicializar y configurar el modulo bluetooth LE
   if(!BLEInit())
      return FALSE;

   debugPrintlnString("Modulo Bluetooth conectado correctamente.");

   return TRUE;
}

void appRun(void) {

   uint8_t pcUartData = 0;

   // ---------- REPETIR POR SIEMPRE --------------------------
   while(TRUE)   {
      // Si leo un dato de una UART_PC lo envio a la UART_BLUETOOTH (bridge)
      if(uartReadByte(UART_PC, &pcUartData )) {
         uartWriteByte(UART_BLUETOOTH, pcUartData);
      }

      // Si presiono TEC1 imprime la lista de comandos AT
      if(!gpioRead(TEC1)) {
         BLEPrintATCommands();
      }

      // Leer el sensor, guardar los resultados en estructura de control,
      // formatear el mensaje recibido y enviarlo al modulo Bluetooth y
      // a la PCUart
      if(TRUE == mpu9250Read()) {

         // Usar LEDB como baliza
         gpioWrite(LEDB, ON);

         // Envio datos giroscopo
         BLESendMsg("G_X","[rad/s]", mpu9250GetGyroX_rads());
         BLESendMsg("G_Y","[rad/s]", mpu9250GetGyroY_rads());
         BLESendMsg("G_Z","[rad/s]", mpu9250GetGyroZ_rads());

         // Envio datos acelerometro
         BLESendMsg("A_X","[m/s2]", mpu9250GetAccelX_mss());
         BLESendMsg("A_Y","[m/s2]", mpu9250GetAccelY_mss());
         BLESendMsg("A_Z","[m/s2]", mpu9250GetAccelZ_mss());

         // Envio datos magnetometro
         BLESendMsg("M_X","[uT]", mpu9250GetMagX_uT());
         BLESendMsg("M_Y","[uT]", mpu9250GetMagY_uT());
         BLESendMsg("M_Z","[uT]", mpu9250GetMagZ_uT());

         // Envio dato temperatura
         BLESendMsg("T","[C]", mpu9250GetTemperature_C());

         gpioWrite(LEDB, OFF);

         delay(1000);
      }
   }
}

void appAssert(void) {

   while(TRUE) {
      gpioWrite(LEDR, ON);
      delay(ASSERT_TICK_INTERVAL);
      gpioWrite(LEDR, OFF);
      delay(ASSERT_TICK_INTERVAL);
   }
}

/*==================[fin del archivo]========================================*/



