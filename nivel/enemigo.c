/*
 * enemigo.c
 *
 *  Created on: Oct 11, 2013
 *      Author: elizabeth
 */

#include "funcionesNivel.h"


void* enemigo(int32_t *idEnemigo) {
	int x=10, y=15;
	int32_t id = (int32_t) idEnemigo;

	// Obtengo parametro del archivo de configuracion
	int32_t sleepEnemigos = configNivelSleepEnemigos();

	rnd(&x, MAXCOLS);
	rnd(&y, MAXROWS);
	log_info(LOGGER, "Creo Enemigo '%c'", id);

	gui_crearEnemigo(id, x, y);
	gui_dibujar();

	while (1) {
		//TODO agregar logica del enemigo
		// Cambiar este movimiento aleatorio por el que corresponde
		rnd(&x, MAXCOLS);
		rnd(&y, MAXROWS);

		gui_mover_personaje(id, x, y );
		gui_dibujar();

		usleep(sleepEnemigos);
	}

	pthread_exit(NULL);
}
