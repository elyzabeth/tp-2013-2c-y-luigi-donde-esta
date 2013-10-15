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

	// pipe() Devuelve en fdPipeToPlanner[0] el descriptor de lectura y en fdPipeToPlanner[1] el descriptor de escritura
	// Plataforma debe escribir en fdPipeToPlanner[1] y el hilo planificador debe leer en fdPipeToPlanner[0]
	if (pipe(nuevoNivel->fdPipe) == -1)
	{
		perror ("tad_nivel-crearNivel: No se puede crear Tuberia de comunicacion.");
		exit (-1);
	}

	return nuevoNivel;
}

void destruirNivel (t_nivel *nivel) {
	close(nivel->fdPipe[0]);
	close(nivel->fdPipe[1]);
	free(nivel);
}


