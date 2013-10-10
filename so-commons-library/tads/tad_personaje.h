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

#define MAXLENNOMBRE 50

#pragma pack(1)
typedef struct coord {
	int32_t x;
	int32_t y;
} t_posicion;
#pragma pack(0)

#pragma pack(1)
typedef struct personaje {
	char nombre[MAXLENNOMBRE+1];
	char id;
	t_posicion posActual;
	int32_t fd;
} t_personaje;
#pragma pack(0)

t_personaje* crearPersonaje (char nombre[MAXLENNOMBRE+1], char id, int32_t posX, int32_t posY, int32_t fd);
void destruirPersonaje (t_personaje * personaje);

#endif /* TAD_PERSONAJE_H_ */
