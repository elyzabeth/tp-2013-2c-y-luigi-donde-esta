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

#pragma pack(1)
typedef struct enemy {
	int32_t id;
	pthread_t tid;
	int32_t fdPipe[2]; // fdPipe[0] de lectura / fdPipe[1] de escritura
} t_enemigo;
#pragma pack(0)

t_enemigo* crearEnemigo(int32_t idEnemigo);
void destruirEnemigo (t_enemigo* enemigo);


#endif /* TAD_ENEMIGO_H_ */
