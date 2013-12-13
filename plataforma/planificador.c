/*
 * planificador.c
 *
 *  Created on: Oct 8, 2013
 *      Author: elizabeth
 */

#include "plataforma.h"

t_personaje* moverPersonajeNuevoAListo(t_planificador *planner);
t_personaje* existePersonajeBloqueadoxRecurso(t_queue *colaPersonajes, char simboloRecurso);
t_personaje* quitarPersonajeColaxId(t_queue *personajesListos, char idPersonaje);
t_personaje* quitarPersonajeColaxFD(t_queue *colaPersonajes, int32_t fdPersonaje);
t_personaje* moverPersonajeListoABloqueado(t_planificador *planner, char idPersonaje);
t_personaje* moverPersonajeBloqueadoAListo( t_planificador *planner, char simboloRecurso );
void planificarPersonaje(t_planificador *planner);
t_personaje* proximoPersonajeRR(t_planificador *planner);
t_personaje* proximoPersonajeSRDF(t_planificador *planner);
int enviarNuevoPersonajeANivel (t_personaje personaje, header_t header, t_planificador *planner );
int enviarMsjTurnoConcedido(t_personaje *personaje, char* nivel);
int recibirCambiosConfiguracion(int32_t fdNivel, header_t header, t_planificador *planner);
int recibirSolicitudUbicacion(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner );
int recibirUbicacionRecursoNivel( header_t header, fd_set *master, t_planificador *planner );
int recibirMovimientoRealizado(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner );
int enviarSolicitudRecursoNivel ( t_personaje personaje, t_planificador *planner );
int recibirSolicitudRecurso(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner );
int recibirRecursoLiberado (int fdPersonaje, header_t header, fd_set *master, t_planificador *planner );
int recibirRecursoConcedido (int fdPersonaje, header_t header, fd_set *master, t_planificador *planner );
int recibirRecursoDenegado (int fdPersonaje, header_t header, fd_set *master, t_planificador *planner );
int recibirRecursoInexistente( header_t header, fd_set *master, t_planificador *planner );
int recibirPlanNivelFinalizado(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner );
int enviarMovimientoRealizadoNivel(header_t *header, t_personaje *personaje, t_planificador *planner);
//int recibirMuertePersonajePlan(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner );
int recibirMuertePersonajePJ2Plan(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner );
int recibirMuertePersonajeNivel2Plan(header_t header, fd_set *master, t_planificador *planner );
void imprimirColas(t_planificador *planner);

extern int errno;

void* planificador(t_planificador *planner) {

	t_personaje *personaje = NULL;
	header_t header;

	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;

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
		//timeout.tv_sec = planner->nivel.retardo * 0.001; /// retardo en Segundos timeout
		timeout.tv_sec = 0;
		timeout.tv_usec = planner->nivel.retardo * 1000; /// retardo en microsegundos timeout

		ret = select(max_desc+1, &read_fds, NULL, NULL, &timeout);

		// ret=-1 ERROR
		// ret= 0 timeout sin actividad en los fd
		if(ret == -1)
		{
			printf(" PLANIFICADOR %s: ERROR en select (%d = %s) (ret= %d) ", planner->nivel.nombre, errno, strerror(errno), ret);
			sleep(1);
			continue;
		}

		if (ret == 0) {
			//log_debug(LOGGER, "PLANIFICADOR %s: timeout", planner->nivel.nombre);
			// Llamar a funcion que realiza la planificacion.
			planificarPersonaje(planner);
		}

		if (ret > 0) {

			for(i = 0; i <= max_desc; i++)
			{

				if (FD_ISSET(i, &read_fds))
				{
					if (i == planner->fdPipe[0])
					{
						//header_t header;
						initHeader(&header);
						log_info(LOGGER, "PLANIFICADOR  %s: Recibo mensaje desde Plataforma por Pipe", planner->nivel.nombre);
						read (planner->fdPipe[0], &header, sizeof(header_t));

						log_debug(LOGGER, "PLANIFICADOR  %s: mensaje recibido '%d'", planner->nivel.nombre, header.tipo);

						if (header.tipo == NUEVO_PERSONAJE) {
							log_debug(LOGGER, "PLANIFICADOR  %s: mensaje recibido '%d' ES NUEVO_PERSONAJE", planner->nivel.nombre, header.tipo);
							personaje = moverPersonajeNuevoAListo(planner);
							agregar_descriptor(personaje->fd, &master, &max_desc);
							enviarNuevoPersonajeANivel(*personaje, header, planner);
						}

						if (header.tipo == IMPRIMIR) {
							log_debug(LOGGER, "PLANIFICADOR  %s: mensaje recibido '%d' ES IMPRIMIR", planner->nivel.nombre, header.tipo);
							// IMPRIMIR COLAS
							imprimirColas(planner);
						}

						if (header.tipo == FINALIZAR) {
							log_debug(LOGGER, "PLANIFICADOR  %s: '%d' ES FINALIZAR", planner->nivel.nombre, header.tipo);
							//cambiarEstadoNivelaFinalizado(planner->nivel.nombre); // lo hace al final
							fin = true;
							quitar_descriptor(planner->fdPipe[0], &master, &max_desc);
							break;
						}

					} else {
						// si NO es un mensaje de plataforma puede ser un nivel o un personaje

						log_debug(LOGGER, "PLANIFICADOR %s: NUEVO MENSAJE en socket %d", planner->nivel.nombre, i);
						initHeader(&header);
						recibir_header(i, &header, &master, &se_desconecto);

						if(se_desconecto)
						{
							//sleep(3);
							// Si se desconecto el Nivel Informo por pantalla y finalizo el hilo.
							if (i == planner->nivel.fdSocket) {
								log_info(LOGGER, "PLANIFICADOR %s: Se desconecto el Nivel (socket %d)", planner->nivel.nombre, i);
								cambiarEstadoNivelaFinalizado(planner->nivel.nombre);
								fin = true;
								quitar_descriptor(planner->fdPipe[0], &master, &max_desc);

								// TODO informar a quien corresponda la desconeccion del nivel.
								//informarDesconeccionAPersonajes();

								quitar_descriptor(i, &master, &max_desc);
								break;

							} else {
								// TODO chequear si se desconecto personaje y borrarlo de las estructuras
								// si es personaje informar al nivel para que lo borre?

								log_debug(LOGGER, "PLANIFICADOR %s: socket %d se desconecto", planner->nivel.nombre, i);
								quitar_descriptor(i, &master, &max_desc);

								// TODO Verificar si el personaje que se desconecta ya esta en estado finalizado !!!
								if (!existePersonajexFDEnFinalizados(i))
									moverPersonajexFDAFinAnormal(i);

								quitarPersonajeColaxFD(planner->personajesListos, i);
								quitarPersonajeColaxFD(planner->personajesBloqueados, i);

								if (planner->personajeEjecutando != NULL && planner->personajeEjecutando->fd == i)
									planner->personajeEjecutando = NULL;

								continue;
							}
						}

						switch(header.tipo) {

							case CAMBIOS_CONFIGURACION: log_info(LOGGER, "PLANIFICADOR %s: CAMBIOS_CONFIGURACION", planner->nivel.nombre);
								recibirCambiosConfiguracion(i, header, planner);
								log_info(LOGGER, "\n\nNUEVOS VALORES\n**************\nAlgoritmo: %s\nQuantum: %d\nRetardo: %d\n", planner->nivel.algoritmo, planner->nivel.quantum, planner->nivel.retardo);
								break;

							case UBICACION_RECURSO: log_info(LOGGER, "PLANIFICADOR %s: UBICACION_RECURSO", planner->nivel.nombre);
								recibirUbicacionRecursoNivel( header, &master, planner);
								break;

							case SOLICITUD_UBICACION: log_info(LOGGER, "PLANIFICADOR %s: SOLICITUD_UBICACION", planner->nivel.nombre);
								recibirSolicitudUbicacion(i, header, &master, planner);
								break;

							case SOLICITUD_RECURSO: log_info(LOGGER, "PLANIFICADOR %s: SOLICITUD_RECURSO", planner->nivel.nombre);
								recibirSolicitudRecurso(i, header, &master, planner);
								break;

							case MOVIMIENTO_REALIZADO: log_info(LOGGER, "PLANIFICADOR %s: MOVIMIENTO_REALIZADO", planner->nivel.nombre);
								recibirMovimientoRealizado(i, header, &master, planner);
								break;

							case RECURSO_CONCEDIDO: log_info(LOGGER, "PLANIFICADOR %s: RECURSO_CONCEDIDO", planner->nivel.nombre);
								recibirRecursoConcedido(i, header, &master, planner);
								break;

							case RECURSO_DENEGADO: log_info(LOGGER, "PLANIFICADOR %s: RECURSO_DENEGADO", planner->nivel.nombre);
								recibirRecursoDenegado(i, header, &master, planner);
								break;

							case RECURSO_LIBERADO: log_info(LOGGER, "PLANIFICADOR %s: RECURSO_LIBERADO", planner->nivel.nombre);
								recibirRecursoLiberado(i, header, &master, planner);
								break;

							case RECURSO_INEXISTENTE: log_info(LOGGER, "PLANIFICADOR %s: RECURSO_INEXISTENTE", planner->nivel.nombre);
								recibirRecursoInexistente( header, &master, planner);
								break;

							case PLAN_NIVEL_FINALIZADO: log_info(LOGGER, "PLANIFICADOR %s: PLAN_NIVEL_FINALIZADO", planner->nivel.nombre);
								recibirPlanNivelFinalizado ( i, header, &master, planner);
								break;

							case MUERTE_PERSONAJE: log_info(LOGGER, "PLANIFICADOR %s: MUERTE_PERSONAJE", planner->nivel.nombre);
								recibirMuertePersonajePJ2Plan( i, header, &master, planner);
								quitar_descriptor(i, &master, &max_desc);
								break;

							case MUERTE_PERSONAJE_XENEMIGO: log_info(LOGGER, "PLANIFICADOR %s: MUERTE_PERSONAJE_XENEMIGO", planner->nivel.nombre);
								recibirMuertePersonajeNivel2Plan( header, &master, planner);
								break;

							case MUERTE_PERSONAJE_XRECOVERY: log_info(LOGGER, "PLANIFICADOR %s: MUERTE_PERSONAJE_XRECOVERY", planner->nivel.nombre);
								recibirMuertePersonajeNivel2Plan( header, &master, planner);
								break;

							case OTRO: log_info(LOGGER, "PLANIFICADOR %s: OTRO", planner->nivel.nombre);
								break;

							default: log_info(LOGGER, "PLANIFICADOR %s: MENSAJE NO RECONOCIDO '%d' ", planner->nivel.nombre, header.tipo);

						}
					}
				}
			}
		}
	}

	log_info(LOGGER, "FINALIZANDO HILO PLANIFICADOR '%s'", planner->nivel.nombre);

	// DEBO DESTRUIR LOS PERSONAJES?? NO SON COMPARTIDOS???? NO DEBERIA SOLO ELIMINAR LAS COLAS???
//	queue_destroy_and_destroy_elements(personajesListos, (void*)destruirPersonaje);
//	queue_destroy_and_destroy_elements(personajesBloqueados, (void*)destruirPersonaje);
	//queue_destroy(personajesListos);
	//queue_destroy(personajesBloqueados);

	moverPersonajesAFinAnormalxNivel(planner->nivel.nombre);

	cambiarEstadoNivelaFinalizado(planner->nivel.nombre);

	pthread_exit(NULL);

}



t_personaje* moverPersonajeNuevoAListo(t_planificador *planner) {
	t_personaje* personaje;
	log_info(LOGGER, "moverPersonajeNuevoAListo Nivel '%s'", planner->nivel.nombre);

	personaje = quitarPersonajeNuevoxNivel(planner->nivel.nombre);
	agregarPersonajeEnJuego(personaje);

	log_info(LOGGER, "Agrego al personaje '%c' a la cola de listos del nivel '%s'.", personaje->id, planner->nivel.nombre);
	queue_push(planner->personajesListos, personaje);

	return personaje;
}


/**
 * @NAME: existePersonajeBloqueadoxRecurso
 * @DESC: Busca si existe un personaje bloqueado por el recurso dado
 * Se le pasa la cola de personajes y el simbolo del recurso que tiene el personaje buscado.
 */
t_personaje* existePersonajeBloqueadoxRecurso(t_queue *colaPersonajes, char simboloRecurso) {
	int tamanio = queue_size(colaPersonajes);
	int i;
	t_personaje *personaje=NULL, *aux=NULL;

	for (i = 0; i < tamanio; i++) {
		aux = queue_pop(colaPersonajes);
		queue_push(colaPersonajes, aux);

		if(personaje==NULL && aux->recurso == simboloRecurso) {
			personaje = aux;
		}
	}

	return personaje;
}


/**
 * @NAME: quitarPersonajeColaxId
 * @DESC: Quita un personaje de la cola de personajes buscandolo por id de personaje (simbolo)
 * Si lo encuentra devuelve el puntero a la estructura t_personaje.
 * Si no lo encuentra devuelve NULL.
 * Se le pasa la cola de personajes y el id del personaje buscado.
 */
t_personaje* quitarPersonajeColaxId(t_queue *colaPersonajes, char idPersonaje) {
	int tamanio = queue_size(colaPersonajes);
	int i;
	t_personaje *personaje=NULL, *aux=NULL;

	for (i = 0; i < tamanio; i++) {
		aux = queue_pop(colaPersonajes);

		if(personaje==NULL && aux->id == idPersonaje) {
			personaje = aux;
		} else {
			queue_push(colaPersonajes, aux);
		}
	}

	return personaje;
}

/**
 * @NAME: quitarPersonajeColaxRecurso
 * @DESC: Quita un personaje de la cola de personajes buscandolo por id de recurso (simbolo)
 * Si lo encuentra devuelve el puntero a la estructura t_personaje.
 * Si no lo encuentra devuelve NULL.
 * Se le pasa la cola de personajes y el simbolo del recurso que tiene el personaje buscado.
 */
t_personaje* quitarPersonajeColaxRecurso(t_queue *colaPersonajes, char simboloRecurso) {
	int tamanio = queue_size(colaPersonajes);
	int i;
	t_personaje *personaje=NULL, *aux=NULL;

	for (i = 0; i < tamanio; i++) {
		aux = queue_pop(colaPersonajes);

		if(personaje==NULL && aux->recurso == simboloRecurso) {
			personaje = aux;
		} else {
			queue_push(colaPersonajes, aux);
		}
	}

	return personaje;
}



/**
 * @NAME: quitarPersonajeColaxFD
 * @DESC: Quita un personaje de la cola de personajes buscandolo por el file descriptor del personaje (socket)
 * Si lo encuentra devuelve el puntero a la estructura t_personaje.
 * Si no lo encuentra devuelve NULL.
 * Se le pasa la cola de personajes y el fd del personaje buscado.
 */
t_personaje* quitarPersonajeColaxFD(t_queue *colaPersonajes, int32_t fdPersonaje) {
	int i;
	int tamanio = queue_size(colaPersonajes);
	t_personaje *personaje=NULL, *aux=NULL;

	for (i = 0; i < tamanio; i++) {
		aux = queue_pop(colaPersonajes);

		if(personaje==NULL && aux->fd == fdPersonaje) {
			personaje = aux;
		} else {
			queue_push(colaPersonajes, aux);
		}
	}

	return personaje;
}

t_personaje* moverPersonajeListoABloqueado( t_planificador *planner, char idPersonaje ) {
	t_personaje *personaje;
	log_debug(LOGGER, "moverPersonajeListoABloqueado '%s': '%c'", planner->nivel.nombre, idPersonaje);

	personaje = quitarPersonajeColaxId(planner->personajesListos, idPersonaje);
	if (NULL != personaje) {
		queue_push(planner->personajesBloqueados, personaje);
		log_info(LOGGER, "Agrego al personaje '%c' a la cola de bloqueados del nivel '%s'.", personaje->id, planner->nivel.nombre);
	} else {
		log_warning(LOGGER, "WARNING! No se encontro el personaje '%c' en la cola de Listos del nivel '%s'!", idPersonaje, planner->nivel.nombre);
	}

	return personaje;
}

t_personaje* moverPersonajeBloqueadoAListo( t_planificador *planner, char simboloRecurso ) {
	t_personaje *personaje;
	log_debug(LOGGER, "moverPersonajeBloqueadoAListo '%s': '%c'", planner->nivel.nombre, simboloRecurso);

	personaje = quitarPersonajeColaxRecurso(planner->personajesBloqueados, simboloRecurso);
	if (NULL != personaje) {
		queue_push(planner->personajesListos, personaje);
		log_info(LOGGER, "Agrego al personaje '%c' a la cola de listos del nivel '%s'.", personaje->id, planner->nivel.nombre);
	} else {
		log_warning(LOGGER, "WARNING! No se encontro ningun personaje bloqueado por el recurso '%c' en la cola de Bloqueado del nivel '%s'!", simboloRecurso, planner->nivel.nombre);
	}

	return personaje;
}

/**
 * @NAME: imprimirPersonajeCola
 * @DESC: Imprime los personajes de la cola de personajes dada.
 * Recibe puntero de tipo t_queue (cola de personajes)
 */
void imprimirPersonajeCola (t_queue *colaPersonajes) {
	int tamanio = queue_size(colaPersonajes);
	int i;
	t_personaje *personaje=NULL;

	for (i = 0; i < tamanio; i++) {
		personaje = queue_pop(colaPersonajes);
		queue_push(colaPersonajes, personaje);
		imprimirPersonajePlat(personaje);
	}

}

void imprimirColas(t_planificador *planner) {

	log_info(LOGGER, "\n\n-- LISTADO Personajes LISTOS en Planificador de %s: ---\n*************************************************", planner->nivel.nombre);
	imprimirPersonajeCola(planner->personajesListos);
	log_info(LOGGER, "\r-- FIN Listado Personajes LISTOS en Planificador de %s: (total: %d) ---\n", planner->nivel.nombre, queue_size(planner->personajesListos));


	log_info(LOGGER, "\n\n-- LISTADO Personajes BLOQUEADOS en Planificador de %s: ---\n*************************************************", planner->nivel.nombre);
	imprimirPersonajeCola(planner->personajesBloqueados);
	log_info(LOGGER, "\r-- FIN Listado Personajes BLOQUEADOS en Planificador de %s: (total: %d) ---\n", planner->nivel.nombre, queue_size(planner->personajesBloqueados));

	if (planner->personajeEjecutando != NULL) {
		log_info(LOGGER, "\n\n -- Personaje En estado EJECUTANDO en Planificador de %s: ---\n*************************************************", planner->nivel.nombre);
		imprimirPersonajePlat(planner->personajeEjecutando);
		log_info(LOGGER, "\n\n -- FIN Personaje En estado EJECUTANDO en Planificador de %s: ---\n*************************************************", planner->nivel.nombre);
	} else {
		log_info(LOGGER, "\n\n -- NO HAY Personaje En estado EJECUTANDO en Planificador de %s: ---\n*************************************************", planner->nivel.nombre);
	}


}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

void planificarPersonaje(t_planificador *planner) {

	if(queue_size(planner->personajesListos)>0) {

		if (planner->personajeEjecutando == NULL || planner->personajeEjecutando->criterio == 0) {
			if ( strcasecmp(planner->nivel.algoritmo, "RR") == 0 )
				planner->personajeEjecutando = proximoPersonajeRR(planner);
			else
				planner->personajeEjecutando = proximoPersonajeSRDF(planner);
		}

		if (enviarMsjTurnoConcedido(planner->personajeEjecutando, planner->nivel.nombre)!=EXITO)
			log_error(LOGGER, "planificarPersonaje %s: ERROR al enviar Turno Concedido a '%s'", planner->nivel.nombre, planner->personajeEjecutando->nombre);
	}
}

t_personaje* proximoPersonajeRR(t_planificador *planner) {
	t_personaje *personaje = NULL;

	// esto es Round Robin
	personaje = queue_pop(planner->personajesListos);
	queue_push(planner->personajesListos, personaje);
	personaje->criterio = planner->nivel.quantum;

	return personaje;
}

/**
 *  Shortest Remaining Distance First - Algoritmo no-expropiativo de planificación que define que el
 * próximo personaje a ser planificado es aquel cuya distancia total al siguiente recurso es más corta.
*/
t_personaje* proximoPersonajeSRDF(t_planificador *planner) {
	t_personaje *personaje=NULL, *aux=NULL;
	int32_t i, srd = 1000;

	// TODO falta agregar elegir personaje segun SRDF (no-expropiativo)
	for(i=0; i < queue_size(planner->personajesListos); i++) {
		aux = queue_pop(planner->personajesListos);
		queue_push(planner->personajesListos, aux);
		if (aux->rd < srd) {
			personaje = aux;
			srd = aux->rd;
		}
	}
	personaje->criterio = personaje->rd;
	return personaje;
}

int recibirCambiosConfiguracion(int fdNivel, header_t header, t_planificador *planner) {
	int ret;
	char *buffer;
	t_nivel nivel;

	buffer = calloc(1, header.largo_mensaje);
	if ((ret = recibir (fdNivel, buffer, header.largo_mensaje)) == EXITO) {
		initNivel(&nivel);
		memcpy(&nivel, buffer, sizeof(t_nivel));

		strcpy(planner->nivel.algoritmo, nivel.algoritmo);
		planner->nivel.quantum = nivel.quantum;
		planner->nivel.retardo = nivel.retardo;
	}

	free(buffer);

	return ret;
}

int enviarSolicitudUbicacionNivel ( header_t header, t_personaje personaje, t_planificador *planner ) {
	int ret;

	log_debug(LOGGER, "enviarSolicitudUbicacionNivel: Envio header (size: %d) SOLICITUD_UBICACION recurso %c al Nivel %s", sizeof(header_t), personaje.recurso, planner->nivel.nombre);

	if (enviar_header(planner->nivel.fdSocket, &header) != EXITO)
	{
		log_error(LOGGER,"enviarSolicitudUbicacionNivel: Error al enviar header SOLICITUD_UBICACION\n\n");
		return WARNING;
	}

	log_debug(LOGGER, "enviarSolicitudUbicacionNivel: Envio t_personaje %d (%s, %c, %d, %d, %d, %s, recurso: %c)", sizeof(t_personaje), personaje.nombre, personaje.id, personaje.posActual.x, personaje.posActual.y, personaje.fd, personaje.nivel, personaje.recurso);
	if ((ret=enviar_personaje(planner->nivel.fdSocket, &personaje)) != EXITO)
	{
		log_error(LOGGER,"enviarSolicitudUbicacionNivel: Error al enviar informacion t_personaje en SOLICITUD_UBICACION\n\n");
		return ret;
	}

	return ret;
}

int recibirUbicacionRecursoNivel( header_t header, fd_set *master, t_planificador *planner ) {
	int ret, se_desconecto;
	t_caja caja;

	log_debug(LOGGER, "recibirUbicacionRecurso: Espero recibir estructura t_caja (size:%d)...", header.largo_mensaje);

	initCaja(&caja);
	if ((ret = recibir_caja(planner->nivel.fdSocket, &caja, master, &se_desconecto))!=EXITO) {
		log_error(LOGGER, "recibirUbicacionRecursoPlanificador: ERROR al recibir t_caja con ubicacion del recurso");
	}

	log_debug(LOGGER, "recibirUbicacionRecurso: Llego: %s (%c) posicion (%d, %d).", caja.RECURSO, caja.SIMBOLO, caja.POSX, caja.POSY);

	// TODO actualizar valor rd (remaining distance)
	//if (strcasecmp(planner->nivel.algoritmo, "SRDF") == 0)
	planner->personajeEjecutando->rd = calcularDistancia(planner->personajeEjecutando->posActual.x,planner->personajeEjecutando->posActual.y, caja.POSX, caja.POSY);

	// Enviar Ubicacion al personaje en Juego...
	ret = enviar_header(planner->personajeEjecutando->fd, &header);
	ret = enviar_caja(planner->personajeEjecutando->fd, &caja);

	return ret;
}


int enviarNuevoPersonajeANivel (t_personaje personaje, header_t header, t_planificador *planner ) {
	int ret;

	log_debug(LOGGER, "%s enviarNuevoPersonajeANivel: Envio estructura personaje (size:%d)...", planner->nivel.nombre, sizeof(t_personaje));

	if ((ret = enviar_header(planner->nivel.fdSocket, &header)) != EXITO) {
			log_error(LOGGER, "ERROR al enviar header_t NUEVO_PERSONAJE al nivel %s", planner->nivel.nombre);
	}
	if ((ret = enviar_personaje(planner->nivel.fdSocket, &personaje)) != EXITO) {
		log_error(LOGGER, "ERROR al enviar t_personaje al nivel %s", planner->nivel.nombre);
	}

	return ret;
}

int recibirSolicitudUbicacion(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner ) {
	int ret, se_desconecto;
	t_personaje personaje;

	log_debug(LOGGER, "Espero recibir estructura personaje (size:%d)...", header.largo_mensaje);
	ret = recibir_personaje(fdPersonaje, &personaje, master, &se_desconecto);
	log_debug(LOGGER, "Llego: %s (%c) al %s. Solicita recurso: '%c'", personaje.nombre, personaje.id, personaje.nivel, personaje.recurso);

	// TODO hacer algo con la info que llega!
	if (planner->personajeEjecutando->id == personaje.id)
		planner->personajeEjecutando->recurso = personaje.recurso;

	// Solicitar Ubicacion al NIVEL...
	ret = enviarSolicitudUbicacionNivel(header, personaje, planner);

	return ret;
}

int enviarSolicitudRecursoNivel ( t_personaje personaje, t_planificador *planner ) {
	int ret;
	header_t header;

	initHeader(&header);
	header.tipo = SOLICITUD_RECURSO;
	header.largo_mensaje = 0;

	log_debug(LOGGER, "enviarSolicitudRecursoNivel: Envio header (size: %d) SOLICITUD_RECURSO recurso %c al Nivel %s", sizeof(header_t), personaje.recurso, planner->nivel.nombre);

	if (enviar_header(planner->nivel.fdSocket, &header) != EXITO)
	{
		log_error(LOGGER,"enviarSolicitudRecursoNivel: Error al enviar header SOLICITUD_RECURSO\n\n");
		return WARNING;
	}

	log_debug(LOGGER, "enviarSolicitudRecursoNivel: Envio t_personaje %d (%s, %c, pos(%d, %d), %d, %s, recurso: %c)", sizeof(t_personaje), personaje.nombre, personaje.id, personaje.posActual.x, personaje.posActual.y, personaje.fd, personaje.nivel, personaje.recurso);
	if ((ret=enviar_personaje(planner->nivel.fdSocket, &personaje)) != EXITO)
	{
		log_error(LOGGER,"enviarSolicitudRecursoNivel: Error al enviar informacion t_personaje en SOLICITUD_RECURSO\n\n");
		return ret;
	}

	return ret;
}

int recibirSolicitudRecurso(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner ) {
	int ret, se_desconecto;
	t_personaje personaje;

	log_debug(LOGGER, "recibirSolicitudRecurso: Espero recibir estructura personaje (size:%d)...", header.largo_mensaje);
	ret = recibir_personaje(fdPersonaje, &personaje, master, &se_desconecto);
	log_debug(LOGGER, "recibirSolicitudRecurso: Llego: %s (%c) al %s. Solicita recurso: '%c'", personaje.nombre, personaje.id, personaje.nivel, personaje.recurso);

	if (planner->personajeEjecutando->id != personaje.id)
		log_error(LOGGER, "\n\nERROR recibirSolicitudRecurso: recibo mensaje SOLICITUD_RECURSO de personaje que no esta en estado EJECUTANDO!!\n\n ");

	planner->personajeEjecutando->recurso = personaje.recurso;

	//	El Planificador moverá a dicho personaje a la cola de bloqueados para ese nivel
	//	y notificará al Nivel de la solicitud. Descartará, si quedara, el quantum de tiempo
	//  restante del Personaje y planificará al siguiente que se encuentre listo.
	moverPersonajeListoABloqueado(planner, personaje.id);
	planner->personajeEjecutando = NULL;

	// Solicitar Recurso al NIVEL...
	ret = enviarSolicitudRecursoNivel(personaje, planner);

	return ret;
}

int enviarMovimientoRealizadoNivel(header_t *header, t_personaje *personaje, t_planificador *planner) {
	int ret;
	ret = enviar_header(planner->nivel.fdSocket, header);
	ret = enviar_personaje(planner->nivel.fdSocket, personaje);
	return ret;
}

int recibirMovimientoRealizado(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner ){
	int ret, se_desconecto;
	t_personaje personaje;

	log_debug(LOGGER, "PLANIFICADOR %s: recibirMovimientoRealizado: Espero recibir estructura personaje (size:%d)...", planner->nivel.nombre, header.largo_mensaje);
	initPersonje(&personaje);
	ret = recibir_personaje(fdPersonaje, &personaje, master, &se_desconecto);
	log_debug(LOGGER, "PLANIFICADOR %s: recibirMovimientoRealizado: Llego: %s (%c) al %s recurso: '%c' se movio a posicion (%d, %d)", planner->nivel.nombre, personaje.nombre, personaje.id, personaje.nivel, personaje.recurso, personaje.posActual.x, personaje.posActual.y);

	// Cuando llega el MSJ de MOVIMIENTO_REALIZADO le resto 1 "quantum" y actualizo su posicion
	if (planner->personajeEjecutando->id == personaje.id) {
		planner->personajeEjecutando->posActual = personaje.posActual;
		planner->personajeEjecutando->criterio--;
		planner->personajeEjecutando->rd--;
	}

	// Enviar MovimientoRealizado al NIVEL...
	ret = enviarMovimientoRealizadoNivel(&header, &personaje, planner);

	return ret;

}

int recibirRecursoLiberado (int fdPersonaje, header_t header, fd_set *master, t_planificador *planner ) {
//	header.id[2] = 'S';
//	return recibirRecursoConcedido ( fdPersonaje, header, master, planner );
	int ret, se_desconecto;
	t_personaje *personaje = NULL;
	t_caja caja;

	// espero recibir estructura t_caja del recurso liberado.
	initCaja(&caja);
	if ((ret = recibir_caja(planner->nivel.fdSocket, &caja, master, &se_desconecto)) != EXITO) {
		log_error(LOGGER, "%s ERROR en recibirRecursoConcedido al recibir t_caja", planner->nivel.nombre);
		return ret;
	}

	// Buscar en la cola de bloqueados si existe un personaje que haya pedido el recurso.
	personaje = existePersonajeBloqueadoxRecurso(planner->personajesBloqueados, caja.SIMBOLO);
	if (personaje != NULL){
		ret = enviarSolicitudRecursoNivel(*personaje, planner);
	}

	return ret;
}

int recibirRecursoConcedido (int fdPersonaje, header_t header, fd_set *master, t_planificador *planner ) {
	int ret, se_desconecto;
	t_personaje *personaje;
	t_caja caja;

	// espero recibir estructura t_caja del recurso concedido.
	initCaja(&caja);
	if ((ret = recibir_caja(planner->nivel.fdSocket, &caja, master, &se_desconecto)) != EXITO) {
		log_error(LOGGER, "%s ERROR en recibirRecursoConcedido al recibir t_caja", planner->nivel.nombre);
		return ret;
	}

	// Buscar en la cola de bloqueados al personaje que pidio el recurso.
	personaje = moverPersonajeBloqueadoAListo(planner, caja.SIMBOLO);

	if (personaje == NULL){
		log_warning(LOGGER, "\n\n%s WARNING recibirRecursoConcedido NO se encontro ningun personaje bloqueado por el recurso '%c'", planner->nivel.nombre, caja.SIMBOLO);
		return WARNING;
	}

	// Envio mensaje de recurso concedido al personaje
	log_debug(LOGGER, "recibirRecursoConcedido: Enviando mensaje de RECURSO_CONCEDIDO '%c' al personaje %s del nivel %s", personaje->recurso, personaje->nombre, personaje->nivel);
	ret = enviar_header(personaje->fd, &header);

	// TODO informo al nivel el personaje que se desbloqueo???
	header.tipo = PERSONAJE_DESBLOQUEADO;
	header.id[0] = personaje->id;
	header.id[1] = personaje->recurso;
	header.largo_mensaje = 0;

	ret = enviar_header(planner->nivel.fdSocket, &header);

	return ret;
}

int recibirRecursoDenegado (int fdPersonaje, header_t header, fd_set *master, t_planificador *planner ) {
	int ret = 0;

	//planner->personajeEjecutando->criterio = 0;
	//moverPersonajeListoABloqueado(planner, planner->personajeEjecutando->id);
	//planner->personajeEjecutando = NULL;

	return ret;
}

int recibirRecursoInexistente( header_t header, fd_set *master, t_planificador *planner ) {
	int ret, maxDesc=0;

	log_debug(LOGGER, "recibirRecursoInexistente: Enviando mensaje de RECURSO_INEXISTENTE '%c' al personaje %s del nivel %s", planner->personajeEjecutando->recurso, planner->personajeEjecutando->nombre, planner->personajeEjecutando->nivel);
	ret = enviar_header(planner->personajeEjecutando->fd, &header);

	// TODO quitar personaje de listados
	quitarPersonajeColaxId(planner->personajesListos, planner->personajeEjecutando->id);

	moverPersonajeAFinAnormal( planner->personajeEjecutando->id, planner->nivel.nombre );

	quitar_descriptor(planner->personajeEjecutando->fd, master, &maxDesc);

	planner->personajeEjecutando = NULL;

	return ret;

}

int recibirPlanNivelFinalizado(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner ) {
	int ret, se_desconecto;
	t_personaje personaje;
	//t_personaje *p;

	initPersonje(&personaje);
	ret = recibir_personaje(fdPersonaje, &personaje, master, &se_desconecto);
	log_debug(LOGGER, "PLANIFICADOR %s: recibirPlanNivelFinalizado: %s (%c) PLAN_NIVEL_FINALIZADO del %s ", planner->nivel.nombre, personaje.nombre, personaje.id, personaje.nivel);

	log_debug(LOGGER, "recibirPlanNivelFinalizado: Enviando mensaje de PLAN_NIVEL_FINALIZADO del personaje %s ('%c') al %s ", personaje.nombre, personaje.id, personaje.nivel);
	if ((ret = enviar_header(planner->nivel.fdSocket, &header)) != EXITO){
		log_error(LOGGER, "ERROR en recibirPlanNivelFinalizado al enviar_header PLAN_NIVEL_FINALIZADO al nivel ");
		return WARNING;
	}

	if ((ret = enviar_personaje(planner->nivel.fdSocket, &personaje)) != EXITO){
		log_error(LOGGER, "ERROR en recibirPlanNivelFinalizado al enviar_personaje PLAN_NIVEL_FINALIZADO al nivel ");
		return WARNING;
	}

	// TODO quitar personaje de listados
	//quitarPersonajeColaxId(planner->personajesListos, personaje.id);

	log_debug(LOGGER, "recibirPlanNivelFinalizado: Moviendo personaje %s ('%c') del %s a Finalizados... ", personaje.nombre, personaje.id, personaje.nivel);
	moverPersonajeAFinalizados(personaje.id, planner->nivel.nombre);

	planner->personajeEjecutando = NULL;
	// TODO Liberar recursos del personaje??

	return ret;
}


/**
 * @NAME: recibirMuertePersonajePJ2Plan
 * @DESC: Recibir Mensaje MUERTE_PERSONAJE desde el personaje
 * Esta situacion puede darse cuando al personaje le llega señal de sigterm y ya no le quedan vidas.
 */
int recibirMuertePersonajePJ2Plan(int fdPersonaje, header_t header, fd_set *master, t_planificador *planner ) {
	int ret, se_desconecto;
	t_personaje personaje;
	//t_personaje *p;

	//recibo el Msj desde el personaje
	initPersonje(&personaje);
	ret = recibir_personaje(fdPersonaje, &personaje, master, &se_desconecto);
	log_debug(LOGGER, "PLANIFICADOR %s: recibirMuertePersonajePJ2Plan: %s (%c) MUERTE_PERSONAJE del %s ", planner->nivel.nombre, personaje.nombre, personaje.id, personaje.nivel);

	// envio Msj al nivel
	log_debug(LOGGER, "recibirMuertePersonajePJ2Plan: Enviando mensaje de MUERTE_PERSONAJE del personaje %s ('%c') al %s ", personaje.nombre, personaje.id, personaje.nivel);
	if ((ret = enviar_header(planner->nivel.fdSocket, &header)) != EXITO){
		log_error(LOGGER, "ERROR en recibirMuertePersonajePJ2Plan al enviar_header MUERTE_PERSONAJE al nivel ");
		return WARNING;
	}

	if ((ret = enviar_personaje(planner->nivel.fdSocket, &personaje)) != EXITO){
		log_error(LOGGER, "ERROR en recibirMuertePersonajePJ2Plan al enviar_personaje MUERTE_PERSONAJE al nivel ");
		return WARNING;
	}

	// quitar personaje de colas del planificador.
	quitarPersonajeColaxId(planner->personajesListos, personaje.id);
	quitarPersonajeColaxId(planner->personajesBloqueados, personaje.id);

	if (planner->personajeEjecutando != NULL && planner->personajeEjecutando->id == personaje.id)
		planner->personajeEjecutando = NULL;

	// TODO quitar personaje de TODAS las listas compartidas
	moverPersonajeAFinAnormal(personaje.id, planner->nivel.nombre);


	return ret;
}

/**
 * @NAME: recibirMuertePersonajeNivel2Plan
 * @DESC: Recibir Mensaje MUERTE_PERSONAJE_XENEMIGO/MUERTE_PERSONAJE_XRECOVERY desde el nivel
 * Esta situacion puede darse cuando un enemigo toca a un personaje o cuando es victima de recovery
 */
int recibirMuertePersonajeNivel2Plan(header_t header, fd_set *master, t_planificador *planner ) {
	int ret, se_desconecto, maxDesc;
	t_personaje personaje;
	t_personaje *pj=NULL;

	initPersonje(&personaje);
	ret = recibir_personaje(planner->nivel.fdSocket, &personaje, master, &se_desconecto);
	log_debug(LOGGER, "PLANIFICADOR %s: recibirMuertePersonajeNivel2Plan: %s (%c) MUERTE_PERSONAJE del %s ", planner->nivel.nombre, personaje.nombre, personaje.id, personaje.nivel);

	// quitar personaje de colas del planificador y de listas compartidas
	// si MUERTE_PERSONAJE_XENEMIGO el personaje debe estar en listos (un enemigo no puede atacar personaje bloqueado)
	// si MUERTE_PERSONAJE_XRECOVERY el personaje debe estar en bloqueados (porque es victima de interbloqueo)

	switch(header.tipo) {

		case MUERTE_PERSONAJE_XENEMIGO:
			pj = quitarPersonajeColaxId(planner->personajesListos, personaje.id);
			if (pj != NULL && planner->personajeEjecutando != NULL && planner->personajeEjecutando->id == pj->id)
				planner->personajeEjecutando = NULL;
			break;

		case MUERTE_PERSONAJE_XRECOVERY:
			pj = quitarPersonajeColaxId(planner->personajesBloqueados, personaje.id);
			break;
	}

	// quitar personaje de listas compartidas
	if ( pj != NULL )
		moverPersonajeAFinAnormal(pj->id, planner->nivel.nombre);

	// enviar Msj al personaje
	log_debug(LOGGER, "recibirMuertePersonajeNivel2Plan: Enviando mensaje de MUERTE_PERSONAJE al personaje %s ('%c') del %s ", personaje.nombre, personaje.id, personaje.nivel);
	if ((ret = enviar_header( pj->fd, &header)) != EXITO) {
		log_error(LOGGER, "ERROR en recibirMuertePersonajeNivel2Plan al enviar_header MUERTE_PERSONAJE al personaje ");
		return WARNING;
	}

	quitar_descriptor(pj->fd, master, &maxDesc);

	return ret;
}

int enviarMsjTurnoConcedido(t_personaje *personaje, char* nivel) {
	int ret;
	header_t header;

	initHeader(&header);
	header.tipo=TURNO_CONCEDIDO;
	header.largo_mensaje=0;

	log_info(LOGGER, "%s: Envio mensaje de TURNO_CONCEDIDO a %s (fd: %d)...", nivel, personaje->nombre, personaje->fd);

	if ((ret =  enviar_header(personaje->fd, &header)) != EXITO ) {
		log_error(LOGGER, "ERROR en enviarMsjTurnoConcedido al enviar_header TURNO_CONCEDIDO al personaje ");
		return ret;
	}

	return ret;
}
