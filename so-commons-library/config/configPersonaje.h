/*
 * configPersonaje.h
 *
 *  Created on: 22/09/2013
 *      Author: arwen
 */

#ifndef CONFIGPERSONAJE_H_
#define CONFIGPERSONAJE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../commons/config.h"
#include "../commons/string.h"
#include "../commons/log.h"
#include "../commons/collections/queue.h"

#include "funciones.h"

#define PATH_CONFIG_PERSONAJE "./personaje.conf"
#define MAXCHARLEN 200
#define MAXOBJXNIVEL 50 // Cantidad maxima de objetivos por nivel.

typedef struct {
		char nivel[200+1];
		char objetivos[50][2];
} t_objetivosxNivel;


// DECLARACION DE FUNCIONES

void levantarArchivoConfiguracionPersonaje ();

const char * configPersonajeNombre();
char configPersonajeSimbolo();
int32_t configPersonajeVidas();
t_queue* configPersonajePlanDeNiveles();
const char * configPersonajePlataforma();
const char * configPersonajePlataformaIp();
int32_t configPersonajePlataformaPuerto();
char * configPersonajeLogPath();
int32_t configPersonajeLogNivel();
int32_t configPersonajeLogConsola();


#endif /* CONFIGPERSONAJE_H_ */
