/*
 * tad_planificador.c
 *
 *  Created on: Oct 16, 2013
 *      Author: elizabeth
 */

#include "tad_planificador.h"

t_planificador* crearPlanificador (t_nivel nivel) {
	t_planificador* nuevoPlanificador;

	nuevoPlanificador = (t_planificador*)calloc(1, sizeof(t_planificador));

	nuevoPlanificador->nivel = nivel;
	nuevoPlanificador->estado = NUEVO;

	// pipe() Devuelve en fdPipe[0] el descriptor de lectura y en fdPipe[1] el descriptor de escritura
	// Plataforma debe escribir en fdPipe[1] y el hilo planificador debe leer en fdPipe[0]
	if (pipe(nuevoPlanificador->fdPipe) == -1)
	{
		perror ("tad_nivel-crearNivel: No se puede crear Tuberia de comunicacion.");
		exit (-1);
	}

	return nuevoPlanificador;
}

void destruirPlanificador (t_planificador* planificador) {
	close(planificador->fdPipe[0]);
	close(planificador->fdPipe[1]);
	free(planificador);
}
