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

#include <stdarg.h>
#include "app.h"
#include "sapi.h"

#include "ff.h"       // <= Biblioteca FAT FS

/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

/* RTC constantes */
#define RTC_INITIAL_YEAR      2018
#define RTC_INITIAL_MONTH     7
#define RTC_INITIAL_DAY       5
#define RTC_INITIAL_HOUR      0
#define RTC_INITIAL_MINUTE    0
#define RTC_INITIAL_SECOND    0
#define RTC_CONFIG_INTERVAL   2000  // In ms

/* SD constantes */
#define SD_TICK_INTERVAL      10 // In ms
#define SD_INIT_RETRIES_MAX   5
#define SD_FILE_NAME          "Log.txt"

/* TASK constantes */
#define TASK_SAMPLING_DELAY   1000  // In ms
#define TASK_BUFF_SIZE        256
#define ASSERT_TICK_INTERVAL  500

/* SD variables */
static FATFS fs;           // <-- FatFs work area needed for each volume
static FIL fp;             // <-- File object needed for each open file


/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[definiciones de funciones internas]=====================*/

/* RTC funciones */
static bool_t _rtcInit(void) {
   bool_t ret;

   /* Estructura del RTC */
   rtc_t rtc = { 0 };

   rtc.year  = RTC_INITIAL_YEAR;
   rtc.month = RTC_INITIAL_MONTH;
   rtc.mday  = RTC_INITIAL_DAY;
   rtc.hour  = RTC_INITIAL_HOUR;
   rtc.min   = RTC_INITIAL_MINUTE;
   rtc.sec   = RTC_INITIAL_SECOND;

   /* Inicializar RTC */
   ret = rtcConfig(&rtc);

   if(ret)
      delay(2000); // El RTC tarda en setear la hora, por eso el delay

   return ret;
}

/* SD funciones */
static void diskTickHook(void *ptr) {
   disk_timerproc();   // Disk timer process
}

static bool_t sdInit(void)
{
   UINT nbytes;
   uint8_t i;

   // Inicializar interfaz SPI0
   spiConfig( SPI0 );

   // Inicializar el conteo de Ticks con resolucion de 10ms,
   // con tickHook diskTickHook
   tickConfig(10);
   tickCallbackSet(diskTickHook, NULL);

   // Configurar el area de trabajo en el disco por defecto
   if(FR_OK == f_mount(&fs, "", 0)) {
      return TRUE;
   }

   return FALSE;
}

/* ADC funciones */
static bool_t _adcInit(void) {
   /* Inicializar AnalogIO */
   /* Posibles configuraciones:
    *    ADC_ENABLE,  ADC_DISABLE,
    *    ADC_ENABLE,  ADC_DISABLE,
    */
   adcConfig(ADC_ENABLE);

   return TRUE;
}

/* MAIN TASK funciones */

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
/*==================[definiciones de funciones externas]=====================*/
bool_t appInit(void) {

   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma
   boardConfig();

   // Inicializar y configurar RTC
   if(!_rtcInit())
      return FALSE;

   // Inicializar y configurar la memoria SD
   if(!sdInit())
      return FALSE;

   // Inicializar y configurar el ADC
   if(!_adcInit())
      return FALSE;

   return TRUE;
}

void appRun(void) {

   uint8_t msg[TASK_BUFF_SIZE];
   uint16_t adcSample_ch1, adcSample_ch2, adcSample_ch3;
   uint32_t msgSize, msgWrittenSize;
   rtc_t rtcTimestamp;
   delay_t samplingDelay;     // Variable de delays no bloqueantes

   /* Inicializar Retardo no bloqueante con tiempo en ms */
   delayConfig(&samplingDelay, TASK_SAMPLING_DELAY);

   // ---------- REPETIR POR SIEMPRE --------------------------
   while(TRUE)
   {
      /* Ejecutar TASK que compone el programa */
      gpioWrite(LEDG, OFF);  // Turn OFF LEDG
      gpioWrite(LEDR, OFF);  // Turn OFF LEDR

      if (delayRead(&samplingDelay)){

         // Muestrear los canales CH1, CH2 y CH3 del ADC
         adcSample_ch1 = adcRead(CH1);
         adcSample_ch2 = adcRead(CH2);
         adcSample_ch3 = adcRead(CH3);

         // Muestrear el RTC
         rtcRead(&rtcTimestamp);

         // Formatear mensaje a almacenar en la memoria SD de la siguiente manera
         //
         // CH1;CH2;CH3;YYYY/MM/DD_hh:mm:ss;
         msgSize = formatVariadicString(msg, sizeof(msg),
                                        "%u;%u;%u;%04u/%02u/%02u_%02u:%02u:%02u\r\n", adcSample_ch1,
                                                                                      adcSample_ch2,
                                                                                      adcSample_ch3,
                                                                                      rtcTimestamp.year,
                                                                                      rtcTimestamp.month,
                                                                                      rtcTimestamp.mday,
                                                                                      rtcTimestamp.hour,
                                                                                      rtcTimestamp.min,
                                                                                      rtcTimestamp.sec);
         // Guardar el mensaje en la memoria SD
         if(msgSize) {

            if(FR_OK == f_open(&fp, SD_FILE_NAME, FA_WRITE | FA_OPEN_APPEND)) {

               f_write(&fp, msg, (UINT) msgSize, (UINT) &msgWrittenSize);
               f_close(&fp);
            }

            if(msgSize == msgWrittenSize) {
               gpioWrite(LEDG, ON);  // Turn ON LEDG if the write operation was successful
            }
            else {
               gpioWrite(LEDR, ON);  // Turn ON LEDR if the write operation was fail
               f_close(&fp);
               break;
            }
         }

         delayConfig(&samplingDelay, TASK_SAMPLING_DELAY);
      }

      sleepUntilNextInterrupt();
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



