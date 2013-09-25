/*
 * funciones.h
 *
 *  Created on: 22/09/2013
 *      Author: arwen
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../commons/string.h"
#include "../commons/log.h"

void separarIpPuerto(char *ipPuerto, char *ip, int32_t *puerto);
t_log_level obtenerLogLevel (char *LOG_NIVEL);

#endif /* FUNCIONES_H_ */
