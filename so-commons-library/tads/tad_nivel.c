/*
 * tad_nivel.c
 *
 *  Created on: 08/10/2013
 *      Author: elyzabeth
 */

#include "tad_nivel.h"

t_nivel * crearNivel ( char *nombre, int32_t fdNivel) {
	t_nivel *nuevoNivel;
	nuevoNivel = (t_nivel*)malloc(sizeof(t_nivel));

	strcpy(nuevoNivel->nombre, nombre);
	nuevoNivel->fdSocket = fdNivel;

	return nuevoNivel;
}

void destruirNivel (t_nivel *nivel) {
	free(nivel);
}


