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

#include "../config/funciones.h"

#pragma pack(1)
typedef struct nivel {
	char nombre[MAXCHARLEN+1];
	char algoritmo[MAXCHARLEN+1];
	int32_t quantum;
	int32_t retardo;
	int32_t fdSocket;
} t_nivel;
#pragma pack(0)

t_nivel * crearNivel ( char * nombre, int32_t fdNivel);
void initNivel(t_nivel *nivel);
void destruirNivel (t_nivel *nivel);
void imprimirNivel (t_nivel *n, t_log *LOGGER);

#endif /* TAD_NIVEL_H_ */
