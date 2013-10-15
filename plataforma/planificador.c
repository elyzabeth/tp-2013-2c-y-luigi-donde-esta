/*
 * planificador.c
 *
 *  Created on: Oct 8, 2013
 *      Author: elizabeth
 */

#include "plataforma.h"

t_personaje* moverPersonajeNuevoAListo(t_queue *personajesListos, char* nivel);

void* planificador(t_nivel *nivel) {

	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;
	char buffer[200];
	t_queue *personajesListos = queue_create();
	//t_queue *personajesBloqueados = queue_create();
	t_personaje *personaje = NULL;
	header_t header;
	int se_desconecto;
//	struct timeval timeout;
//	timeout.tv_sec = 50000;
//	timeout.tv_usec = 0;

	FD_ZERO(&master);
	agregar_descriptor(nivel->fdPipe[0], &master, &max_desc);

	//int ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);

	log_info(LOGGER, "PLANIFICADOR %s: Iniciado.", nivel->nombre);
	while(1) {
		FD_ZERO (&read_fds);
		read_fds = master;

		//ret = select(max_desc+1, &read_fds, NULL, NULL, &timeout);
		ret = select(max_desc+1, &read_fds, NULL, NULL, NULL);
		log_debug(LOGGER, "PLANIFICADOR %s: ret: %d ", nivel->nombre, ret);
		// ret=-1 ERROR
		// ret= 0 timeout sin actividad en los fd
		if(ret == -1)
		{
			printf("PLANIFICADOR %s: ERROR en select.", nivel->nombre);
		}

		if (ret == 0) {
			log_debug(LOGGER, "PLANIFICADOR %s: timeout", nivel->nombre);
			continue;
		}

		if (ret > 0) {
			for(i = 0; i <= max_desc; i++)
			{

				if (FD_ISSET(i, &read_fds) && (i == nivel->fdPipe[0]))
				{
					log_info(LOGGER, "PLANIFICADOR  %s: Recibo mensaje desde Plataforma por Pipe", nivel->nombre);
					read (nivel->fdPipe[0], buffer, 15);
					//recibir_header(nivel->fdPipe[0], &header, &master, &se_desconecto);
					//log_debug(LOGGER, "PLANIFICADOR  %s: mensaje recibido '%c'", nivel->nombre, header.tipo);
					log_debug(LOGGER, "PLANIFICADOR  %s: mensaje recibido '%s'", nivel->nombre, buffer);
					personaje = moverPersonajeNuevoAListo(personajesListos, nivel->nombre);
					agregar_descriptor(personaje->fd, &master, &max_desc);
				}

				if (FD_ISSET(i, &read_fds) && (i != nivel->fdPipe[0]))
				{
					log_info(LOGGER, "PLANIFICADOR %s: NUEVO MENSAJE DE PERSONAJE en socket %d", nivel->nombre, i);
					recibir_header(i, &header, &master, &se_desconecto);
					log_debug(LOGGER, "PLANIFICADOR %s: socket %d recibo: %c", nivel->nombre, i, header.tipo);

					if(se_desconecto)
					{
						log_info(LOGGER, "Se desconecto el socket %d", i);
						// TODO chequear si se desconecto personaje o nivel y borrarlo de las estructuras
						// si es personaje informar al nivel para que lo borre?
					}

				}
			}
		}
		// TODO QUITAR ESTE SLEEP!!
		sleep(5);
	}


	pthread_exit(NULL);
}

t_personaje* moverPersonajeNuevoAListo(t_queue *personajesListos, char* nivel) {
	t_personaje* personaje;
	log_info(LOGGER, "moverPersonajeNuevoAListo '%s'.", nivel);

	personaje = quitarPersonajeNuevoxNivel(nivel);
	agregarPersonajeEnJuego(personaje);

	log_info(LOGGER, "Agrego al personaje '%c' a la cola de listos del nivel '%s'.", personaje->id, nivel);
	queue_push(personajesListos, personaje);

	return personaje;
}
