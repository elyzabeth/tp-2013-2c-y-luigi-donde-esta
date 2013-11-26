/*
 * tad_enemigo.c
 *
 *  Created on: Oct 17, 2013
 *      Author: elizabeth
 */

#include "tad_enemigo.h"

t_hiloEnemigo* crearHiloEnemigo(int32_t idEnemigo) {
	t_hiloEnemigo *nuevoHiloEnemigo;
	nuevoHiloEnemigo = calloc(1, sizeof(t_hiloEnemigo));

	nuevoHiloEnemigo->enemigo.id = idEnemigo;
	nuevoHiloEnemigo->id = idEnemigo;

	// pipe() Devuelve en fdPipe[0] el descriptor de lectura y en fdPipe[1] el descriptor de escritura
	// NivelMain debe escribir en fdPipe[1] y el hilo enemigo debe leer en fdPipe[0]
	if (pipe(nuevoHiloEnemigo->fdPipe) == -1)
	{
		perror ("tad_enemigo-crearEnemigo: No se puede crear Tuberia de comunicacion.");
		exit (-1);
	}

	if (pipe(nuevoHiloEnemigo->fdPipeE2N) == -1)
	{
		perror ("tad_enemigo-crearEnemigo: No se puede crear Tuberia de comunicacion.");
		exit (-1);
	}

	return nuevoHiloEnemigo;
}

void destruirHiloEnemigo (t_hiloEnemigo* enemigo) {
	close(enemigo->fdPipe[0]);
	close(enemigo->fdPipe[1]);
	close(enemigo->fdPipeE2N[0]);
	close(enemigo->fdPipeE2N[1]);
	free(enemigo);
}
