/*
 * planificador.c
 *
 *  Created on: Oct 8, 2013
 *      Author: elizabeth
 */

#include "plataforma.h"

t_personaje* moverPersonajeNuevoAListo(t_queue *personajesListos, char* nivel);
t_personaje* quitarPersonajeColaxId(t_queue *personajesListos, char idPersonaje);
t_personaje* moverPersonajeListoABloqueado(t_queue *personajesListos, t_queue *personajesBloqueados, char idPersonaje, char* nivel);
//void plan_signal_callback_handler(int signum);


void* planificador(t_planificador *planner) {

	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;

	t_queue *personajesListos = queue_create();
	t_queue *personajesBloqueados = queue_create();
	t_personaje *personaje = NULL;
	header_t header;
	int se_desconecto;
	int fin = 0;
	struct timeval timeout;

	cambiarEstadoNivelaCorriendo(planner->nivel.nombre);
	log_info(LOGGER, "PLANIFICADOR %s: Iniciado.", planner->nivel.nombre);

	FD_ZERO(&master);

	// Agrego descriptor del Pipe con plataforma.
	agregar_descriptor(planner->fdPipe[0], &master, &max_desc);

	// Agrego descriptor del socket del nivel
	agregar_descriptor(planner->nivel.fdSocket, &master, &max_desc);

	while(!fin) {

		FD_ZERO (&read_fds);
		read_fds = master;
		timeout.tv_sec = planner->nivel.retardo; /// retardo en segundos timeout
		timeout.tv_usec = 0;

		ret = select(max_desc+1, &read_fds, NULL, NULL, &timeout);
		//ret = select(max_desc+1, &read_fds, NULL, NULL, NULL);
		log_debug(LOGGER, "PLANIFICADOR %s: ret: %d ", planner->nivel.nombre, ret);
		// ret=-1 ERROR
		// ret= 0 timeout sin actividad en los fd
		if(ret == -1)
		{
			printf("PLANIFICADOR %s: ERROR en select.", planner->nivel.nombre);
			sleep(1);
			continue;
		}

		if (ret == 0) {
			log_debug(LOGGER, "PLANIFICADOR %s: timeout", planner->nivel.nombre);
			// TODO Llamar a funcion que realiza la planificacion.
			// planificarPersonaje();
			continue;
		}

		if (ret > 0) {
			for(i = 0; i <= max_desc; i++)
			{

				if (FD_ISSET(i, &read_fds) && (i == planner->fdPipe[0]))
				{
					header_t header;
					log_info(LOGGER, "PLANIFICADOR  %s: Recibo mensaje desde Plataforma por Pipe", planner->nivel.nombre);
					read (planner->fdPipe[0], &header, sizeof(header_t));

					log_debug(LOGGER, "PLANIFICADOR  %s: mensaje recibido '%d'", planner->nivel.nombre, header.tipo);
					if (header.tipo == NUEVO_PERSONAJE) {
						log_debug(LOGGER, "PLANIFICADOR  %s: mensaje recibido '%d' ES NUEVO_PERSONAJE", planner->nivel.nombre, header.tipo);
						personaje = moverPersonajeNuevoAListo(personajesListos, planner->nivel.nombre);
						agregar_descriptor(personaje->fd, &master, &max_desc);
					}
					if (header.tipo == FINALIZAR) {
						log_debug(LOGGER, "PLANIFICADOR  %s: '%d' ES FINALIZAR", planner->nivel.nombre, header.tipo);
						cambiarEstadoNivelaFinalizado(planner->nivel.nombre);
						fin = true;
						FD_CLR(planner->fdPipe[0], &master);
						break;
					}

				}

				if (FD_ISSET(i, &read_fds) && (i != planner->fdPipe[0]))
				{
					log_info(LOGGER, "PLANIFICADOR %s: NUEVO MENSAJE en socket %d", planner->nivel.nombre, i);
					recibir_header(i, &header, &master, &se_desconecto);

					if(se_desconecto)
					{
						// Si se desconecto el Nivel Informo por pantalla y finalizo el hilo.
						if (i == planner->nivel.fdSocket) {
							log_info(LOGGER, "PLANIFICADOR %s: Se desconecto el Nivel (socket %d)", planner->nivel.nombre, i);
							cambiarEstadoNivelaFinalizado(planner->nivel.nombre);
							fin = true;
							FD_CLR(planner->nivel.fdSocket, &master);
							//informarDesconeccionPersonajes();
							break;
						} else {
							// TODO chequear si se desconecto personaje y borrarlo de las estructuras
							// si es personaje informar al nivel para que lo borre?
							log_debug(LOGGER, "PLANIFICADOR %s: socket %d recibo: %c", planner->nivel.nombre, i, header.tipo);
						}
					}

//					switch(header.tipo){
//						case SOLICITA_RECURSO:
//					}

				}
			}
		}
	}

	log_info(LOGGER, "FINALIZANDO HILO PLANIFICADOR '%s'", planner->nivel.nombre);

	// DEBO DESTRUIR LOS PERSONAJES?? NO SON COMPARTIDOS???? NO DEBERIA SOLO ELIMINAR LAS COLAS???
//	queue_destroy_and_destroy_elements(personajesListos, (void*)destruirPersonaje);
//	queue_destroy_and_destroy_elements(personajesBloqueados, (void*)destruirPersonaje);
	queue_destroy(personajesListos);
	queue_destroy(personajesBloqueados);
	moverPersonajesAFinAnormalxNivel(planner->nivel.nombre);

	pthread_exit(NULL);

}



t_personaje* moverPersonajeNuevoAListo(t_queue *personajesListos, char* nivel) {
	t_personaje* personaje;
	log_info(LOGGER, "moverPersonajeNuevoAListo Nivel '%s'", nivel);

	personaje = quitarPersonajeNuevoxNivel(nivel);
	agregarPersonajeEnJuego(personaje);

	log_info(LOGGER, "Agrego al personaje '%c' a la cola de listos del nivel '%s'.", personaje->id, nivel);
	queue_push(personajesListos, personaje);

	return personaje;
}


/**
 * @NAME: quitarPersonajeColaxId
 * @DESC: Quita un personaje de la cola buscandolo por id de personaje (simbolo)
 * Si lo encuentra devuelve el puntero a la estructura t_personaje.
 * Si no lo encuentra devuelve NULL.
 */
t_personaje* quitarPersonajeColaxId(t_queue *personajesListos, char idPersonaje) {
	int tamanio = queue_size(personajesListos);
	int i;
	t_personaje *personaje = NULL;

	for (i = 0; i < tamanio; i++) {
		personaje = queue_pop(personajesListos);

		if(!personaje->id == idPersonaje){
			queue_push(personajesListos, personaje);
			personaje = NULL;
		}
	}

	return personaje;
}

t_personaje* moverPersonajeListoABloqueado(t_queue *personajesListos, t_queue *personajesBloqueados, char idPersonaje, char* nivel) {
	t_personaje *personaje;
	log_info(LOGGER, "moverPersonajeListoABloqueado '%s': '%c'", nivel, idPersonaje);

	personaje = quitarPersonajeColaxId(personajesListos, idPersonaje);
	if (NULL != personaje) {
		queue_push(personajesBloqueados, personaje);
		log_info(LOGGER, "Agrego al personaje '%c' a la cola de bloqueados del nivel '%s'.", personaje->id, nivel);
	} else {
		log_warning(LOGGER, "WARNING! No se encontro el personaje '%c' en la cola de Listos del nivel '%s'!", idPersonaje, nivel);
	}

	return personaje;
}
