/*
 * tad_enemigo.h
 *
 *  Created on: Oct 17, 2013
 *      Author: elizabeth
 */

#ifndef TAD_ENEMIGO_H_
#define TAD_ENEMIGO_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tad_posicion.h"

#pragma pack(1)
typedef struct {
	int32_t id;
	t_posicion posicionActual;
	t_posicion posicionEleSiguiente;
	int32_t moverPorX;//flag para q no se muevan por el mismo eje
} t_enemigo;
#pragma pack(0)

#pragma pack(1)
typedef struct enemy {
	int32_t id;
	t_enemigo enemigo;
	pthread_t tid;
	int32_t fdPipe[2]; // fdPipe[0] de lectura del enemigo / fdPipe[1] de escritura del nivel
	int32_t fdPipeE2N[2]; // fdPipe[0] de lectura del nivel / fdPipe[1] de escritura del enemigo
} t_hiloEnemigo;
#pragma pack(0)

t_hiloEnemigo* crearHiloEnemigo(int32_t idEnemigo);
void destruirHiloEnemigo (t_hiloEnemigo* enemigo);


#endif /* TAD_ENEMIGO_H_ */
