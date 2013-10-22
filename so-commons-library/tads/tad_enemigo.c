/*
 * tad_enemigo.c
 *
 *  Created on: Oct 17, 2013
 *      Author: elizabeth
 */

#include "tad_enemigo.h"

t_hiloEnemigo* crearEnemigo(int32_t idEnemigo) {
	t_hiloEnemigo *nuevoEnemigo;
	nuevoEnemigo = calloc(1, sizeof(t_hiloEnemigo));

	nuevoEnemigo->id = idEnemigo;

	// pipe() Devuelve en fdPipe[0] el descriptor de lectura y en fdPipe[1] el descriptor de escritura
	// NivelMain debe escribir en fdPipe[1] y el hilo enemigo debe leer en fdPipe[0]
	if (pipe(nuevoEnemigo->fdPipe) == -1)
	{
		perror ("tad_enemigo-crearEnemigo: No se puede crear Tuberia de comunicacion.");
		exit (-1);
	}

	return nuevoEnemigo;
}

void destruirEnemigo (t_hiloEnemigo* enemigo) {
	close(enemigo->fdPipe[0]);
	close(enemigo->fdPipe[1]);
	free(enemigo);
}
