/*
 * enemigo.c
 *
 *  Created on: Oct 11, 2013
 *      Author: elizabeth
 */

#include "funcionesNivel.h"


void* enemigo (t_enemigo *enemy) {

	int32_t id = (int32_t) enemy->id;
	int32_t sleepEnemigos;
	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;
	int x=10, y=15;
	int fin = false;
	struct timeval timeout;

	log_info(LOGGER, "Enemigo '%c' Iniciado.", id);

	// Obtengo parametro del archivo de configuracion
	sleepEnemigos = configNivelSleepEnemigos();

	FD_ZERO(&master);

	// Agrego descriptor del Pipe con Nivel.
	agregar_descriptor(enemy->fdPipe[0], &master, &max_desc);

	rnd(&x, MAXCOLS);
	rnd(&y, MAXROWS);
	gui_crearEnemigo(id, x, y);
	gui_dibujar();

	while (!fin) {

		FD_ZERO (&read_fds);
		read_fds = master;
		timeout.tv_sec = 0; /// retardo en segundos timeout
		timeout.tv_usec = sleepEnemigos; //retardo en microsegundos timeout

		ret = select(max_desc+1, &read_fds, NULL, NULL, &timeout);
		if(ret == -1) {
			printf("Enemigo '%c': ERROR en select.", enemy->id);
			sleep(1);
			continue;
		}
		if (ret == 0) {
			//log_debug(LOGGER, "Enemigo '%c': timeout", enemy->id);

			//TODO agregar logica del enemigo
			// Cambiar este movimiento aleatorio por el que corresponde
			rnd(&x, MAXCOLS);
			rnd(&y, MAXROWS);

			gui_mover_personaje(id, x, y );
			gui_dibujar();

		}
		if (ret > 0) {
			for(i = 0; i <= max_desc; i++)
			{

				if (FD_ISSET(i, &read_fds) && (i == enemy->fdPipe[0]))
				{
					header_t header;
					log_info(LOGGER, "Enemigo '%c': Recibo mensaje desde Nivel por Pipe", enemy->id);
					read (enemy->fdPipe[0], &header, sizeof(header_t));

					log_debug(LOGGER, "Enemigo '%c': mensaje recibido '%d'", enemy->id, header.tipo);
					if (header.tipo == FINALIZAR) {
						log_debug(LOGGER, "Enemigo '%c': '%d' ES FINALIZAR", enemy->id, header.tipo);
						fin = true;
						break;
					}

				}
			}
		}

	}

	log_info(LOGGER, "FINALIZANDO ENEMIGO '%c' \n", id);
	gui_borrarItem(id);

	pthread_exit(NULL);
}
