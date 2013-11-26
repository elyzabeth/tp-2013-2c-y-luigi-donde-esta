/*
 * tad_personaje.h
 *
 *  Created on: 07/10/2013
 *      Author: elyzabeth
 */

#ifndef TAD_PERSONAJE_H_
#define TAD_PERSONAJE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../commons/log.h"
#include "tad_posicion.h"

#define MAXLENNOMBRE 50


#pragma pack(1)
typedef struct personaje {
	char nombre[MAXLENNOMBRE+1];
	char id;
	t_posicion posActual;
	int32_t fd;
	char nivel[MAXLENNOMBRE+1];
	char recurso;
	int32_t criterio;
	int32_t rd;
} t_personaje;
#pragma pack(0)

t_personaje* crearPersonaje (char nombre[MAXLENNOMBRE+1], char id, int32_t posX, int32_t posY, int32_t fd, char* nivel);
t_personaje* crearPersonajeDesdePersonaje (t_personaje personaje);
t_personaje* crearPersonajeVacio ();
void initPersonje(t_personaje *personaje);
void reiniciarPersonje(t_personaje *personaje);
void destruirPersonaje (t_personaje * personaje);

void imprimirPersonaje (t_personaje* p, t_log *LOGGER);

#endif /* TAD_PERSONAJE_H_ */
