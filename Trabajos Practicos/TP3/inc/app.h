/*
 * app.h
 *
 *  Created on: 5/7/2018
 *      Author: rodrigo
 */

/*============================================================================
 * Autor:   Rodrigo Tirapegui
 * Licencia:
 * Fecha:   22/3/2018
 *===========================================================================*/

#ifndef _APP_H_
#define _APP_H_

/*==================[inclusiones]============================================*/

#include "sapi.h"

/*==================[c++]====================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[tipos de datos declarados por el usuario]===============*/

/*==================[declaraciones de datos externos]========================*/

/*==================[declaraciones de funciones externas]====================*/

/**
 * \brief Inicializa todos los modulos de la aplicacion
  *
 * \param No recibe par�metros
 *
 * \return bool_t TRUE si la inicializacion fue corrcta. FALSE en caso contrario
 */
bool_t appInit(void);


/**
 * \brief Ejecuta el superloop principal de la aplicacion
 *
 * \param No recibe parametros
 *
 * \return void No devuleve nada
 */
void appRun(void);

/**
 * \brief Ejecuta un superloop en el cual se indica condicion de falla de la
 * aplicacion
 *
 * \param No recibe parametros
 *
 * \return void No devuleve nada
 */
void appAssert(void);

/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_H_ */
