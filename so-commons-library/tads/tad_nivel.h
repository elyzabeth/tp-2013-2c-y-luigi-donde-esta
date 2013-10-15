/*
 * tad_nivel.h
 *
 *  Created on: 08/10/2013
 *      Author: elyzabeth
 */

#ifndef TAD_NIVEL_H_
#define TAD_NIVEL_H_

#include <stdlib.h>
#include <string.h>

#pragma pack(1)
typedef struct nivel {
	char nombre[50+1];
	int32_t fdSocket;
	int32_t fdPipe[2]; // fdPipe[0] de lectura/ fdPipe[1] de escritura
} t_nivel;
#pragma pack(0)

t_nivel * crearNivel ( char * nombre, int32_t fdNivel);
void destruirNivel (t_nivel *nivel);

#endif /* TAD_NIVEL_H_ */
