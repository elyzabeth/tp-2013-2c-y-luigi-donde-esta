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

#define PATH_CONFIG_NIVEL "./nivel.conf"
#define PATH_CONFIG_PERSONAJE "./personaje.conf"
#define PATH_CONFIG_PLATAFORMA "./plataforma.conf"

#define MAXCHARLEN 200

void separarIpPuerto(char *ipPuerto, char *ip, int32_t *puerto);
t_log_level obtenerLogLevel (char *LOG_NIVEL);
void quitarCorchetes (char *to, char *from);


#endif /* FUNCIONES_H_ */
