/*
 * listas_compartidas.c
 *
 *  Created on: Oct 8, 2013
 *      Author: elizabeth
 */

#include "plataforma.h"

/**
 * @NAME: agregarPersonajeNuevo
 * @DESC: Agrega un personaje la lista compartida ListaPersonajesNuevos
 * usando semaforo mutex.
 */
void agregarPersonajeNuevo(t_personaje* personaje) {

	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	list_add(listaPersonajesNuevos, personaje);
	//plataforma.personajes_en_juego++;
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);
	imprimirListaPersonajesNuevos();
}


/**
 * @NAME: agregarPersonajeEnJuego
 * @DESC: Agrega un personaje a la lista compartida listaPersonajesEnJuego
 * usando semaforo mutex.
 */
void agregarPersonajeEnJuego(t_personaje* personaje) {

	pthread_mutex_lock (&mutexListaPersonajesEnJuego);
	list_add(listaPersonajesEnJuego, personaje);
	plataforma.personajes_en_juego++;
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);
	imprimirListaPersonajesEnJuego();
}


/**
 * @NAME: agregarPersonajeFinalizado
 * @DESC: Agrega un personaje la lista compartida ListaPersonajesFinalizados
 * usando semaforo mutex.
 */
void agregarPersonajeFinalizado(t_personaje* personaje) {

	pthread_mutex_lock (&mutexListaPersonajesFinalizados);
	list_add(listaPersonajesFinalizados, personaje);
	pthread_mutex_unlock (&mutexListaPersonajesFinalizados);
}

/**
 * @NAME: agregarPersonajeFinAnormal
 * @DESC: Agrega un personaje la lista compartida ListaPersonajesFinAnormal
 * usando semaforo mutex.
 */
void agregarPersonajeFinAnormal(t_personaje* personaje) {

	pthread_mutex_lock (&mutexListaPersonajesFinAnormal);
	list_add(listaPersonajesFinAnormal, personaje);
	//plataforma.personajes_en_juego--; ???
	pthread_mutex_unlock (&mutexListaPersonajesFinAnormal);
}

_Bool existePersonajexFDEnFinalizados(int fdPersonaje) {

	pthread_mutex_lock (&mutexListaPersonajesFinalizados);
	t_personaje *personaje;
	_Bool _buscarxfd(t_personaje* p) {
		return (p->fd == fdPersonaje);
	}
	personaje = list_find(listaPersonajesFinalizados, (void*)_buscarxfd);
	pthread_mutex_unlock (&mutexListaPersonajesFinalizados);

	return (personaje != NULL );
}

t_personaje* quitarPersonajeNuevoxNivel(char* nivel) {

	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	t_personaje *personaje;

	bool _buscar_x_nivel(t_personaje *p) {
		return (strcasecmp(p->nivel, nivel)==0);
	}
	personaje = list_remove_by_condition(listaPersonajesNuevos, (void*)_buscar_x_nivel);
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);
	imprimirListaPersonajesNuevos();
	return personaje;
}


t_personaje* quitarPersonajeNuevoxFD(int32_t fdPersonaje) {
	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	t_personaje *personaje;

	bool _buscar_x_fd(t_personaje *p) {
		return (p->fd == fdPersonaje);
	}
	personaje = list_remove_by_condition(listaPersonajesNuevos, (void*)_buscar_x_fd);
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);
	imprimirListaPersonajesNuevos();

	return personaje;
}


t_personaje* quitarPersonajeEnJuegoxFD(int32_t fdPersonaje) {

	pthread_mutex_lock (&mutexListaPersonajesEnJuego);
	t_personaje *personaje;

	bool _buscar_x_fd(t_personaje *p) {
		return (p->fd == fdPersonaje);
	}

	personaje = list_remove_by_condition(listaPersonajesEnJuego, (void*)_buscar_x_fd);
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);
	imprimirListaPersonajesEnJuego();

	return personaje;
}


t_personaje* quitarPersonajeNuevoxNivelxId (char* nivel, char idPersonaje) {

	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	t_personaje *personaje;

	bool _buscar_xnivel_xid(t_personaje *p) {
		return (strcasecmp(p->nivel, nivel)==0 && p->id == idPersonaje);
	}
	personaje = list_remove_by_condition(listaPersonajesNuevos, (void*)_buscar_xnivel_xid);
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);
	imprimirListaPersonajesNuevos();

	return personaje;
}


t_personaje* quitarPersonajeEnJuegoxNivelxId (char* nivel, char idPersonaje) {

	pthread_mutex_lock (&mutexListaPersonajesEnJuego);
	t_personaje *personaje;

	bool _buscar_xnivel_xid(t_personaje *p) {
		//log_debug(LOGGER, "quitarPersonajeEnJuegoxNivelxId: p->nivel = nivel ? "p->nivel, nivel, strcasecmp(p->nivel, nivel));
		return (strcasecmp(p->nivel, nivel)==0 && p->id == idPersonaje);
	}
	personaje = list_remove_by_condition(listaPersonajesEnJuego, (void*)_buscar_xnivel_xid);
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);
	imprimirListaPersonajesEnJuego();

	return personaje;
}


t_personaje* quitarPersonajeFinalizadoxNivelxId (char* nivel, char idPersonaje) {

	pthread_mutex_lock (&mutexListaPersonajesFinalizados);
	t_personaje *personaje;

	bool _buscar_xnivel_xid(t_personaje *p) {
		return (strcasecmp(p->nivel, nivel)==0 && p->id == idPersonaje);
	}
	personaje = list_remove_by_condition(listaPersonajesFinalizados, (void*)_buscar_xnivel_xid);
	pthread_mutex_unlock (&mutexListaPersonajesFinalizados);
	imprimirListaPersonajesFinalizados();

	return personaje;
}


t_personaje* moverPersonajexFDAFinAnormal(int32_t fdPersonaje) {

	t_personaje *personaje;

	personaje = quitarPersonajeNuevoxFD(fdPersonaje);
	if (personaje == NULL)
		personaje = quitarPersonajeEnJuegoxFD(fdPersonaje);

	if (personaje != NULL)
		agregarPersonajeFinAnormal(personaje);

	return personaje;
}

void moverPersonajeAFinalizados(char idPersonaje, char *nivel) {
	//TODO!!!
	t_personaje *personaje = NULL;

	log_info(LOGGER, "moverPersonajeAFinalizados: Moviendo al personaje %c del %s a Finalizados", idPersonaje, nivel);
	personaje = quitarPersonajeEnJuegoxNivelxId(nivel, idPersonaje);

	if (personaje != NULL) {
		agregarPersonajeFinalizado(personaje);
	} else {
		log_error(LOGGER, "moverPersonajeAFinalizados: ERROR No se encontro al personaje %c del %s en el listado de personajes en Juego!!", idPersonaje, nivel);
	}

	imprimirListaPersonajesNuevos();
	imprimirListaPersonajesEnJuego();
	imprimirListaPersonajesFinalizados();

}

void moverPersonajeAFinAnormal (char idPersonaje, char *nivel) {
	//TODO buscar tambien en lista de finalizados!!!
	t_personaje *personaje = NULL;

	personaje = quitarPersonajeNuevoxNivelxId(nivel, idPersonaje);

	if (personaje == NULL)
		personaje = quitarPersonajeEnJuegoxNivelxId(nivel, idPersonaje);

	if (personaje == NULL)
			personaje = quitarPersonajeFinalizadoxNivelxId(nivel, idPersonaje);

	if (personaje != NULL)
		agregarPersonajeFinAnormal(personaje);

}

/**
 * @NAME: moverPersonajesAFinAnormalxNivel
 * @DESC: Mueve todos los personajes asociados a un nivel de todas las listas a la lista compartida ListaPersonajesFinAnormal
 * usando semaforo mutex.
 * Se utiliza cuando un nivel tuvo un fin anormal.
 */
void moverPersonajesAFinAnormalxNivel (char *nivel) {
	t_list *aux;
	t_list *aux2;
	log_info(LOGGER, "moverPersonajesAFinAnormal Nivel '%s'", nivel);

	bool _buscar_x_nivel(t_personaje *p) {
			return (strcasecmp(p->nivel, nivel)==0);
	}

	pthread_mutex_lock (&mutexListaPersonajesEnJuego);
	aux = list_filter(listaPersonajesEnJuego, (void*)_buscar_x_nivel);
	list_remove_by_condition(listaPersonajesEnJuego, (void*)_buscar_x_nivel);
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);

	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	aux2 = list_filter(listaPersonajesNuevos, (void*)_buscar_x_nivel);
	list_remove_by_condition(listaPersonajesNuevos, (void*)_buscar_x_nivel);
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);

	pthread_mutex_lock (&mutexListaPersonajesFinAnormal);
	list_add_all(listaPersonajesFinAnormal, aux);
	list_add_all(listaPersonajesFinAnormal, aux2);
	pthread_mutex_unlock (&mutexListaPersonajesFinAnormal);

	list_destroy(aux);
	list_destroy(aux2);
}


bool existeNivel(char* nivel) {
	pthread_mutex_lock (&mutexListaNiveles);
	bool existe;
	existe = dictionary_has_key(listaNiveles, nivel);
	pthread_mutex_unlock (&mutexListaNiveles);
	return existe;
}

t_planificador* obtenerNivel(char* nivel) {
	pthread_mutex_lock (&mutexListaNiveles);
	t_planificador *planner = NULL;
	planner = dictionary_get(listaNiveles, nivel);
	pthread_mutex_unlock (&mutexListaNiveles);
	return planner;
}

t_estado obtenerEstadoNivel(char* nivel) {
	pthread_mutex_lock (&mutexListaNiveles);
	t_planificador *planner = NULL;
	planner = dictionary_get(listaNiveles, nivel);
	pthread_mutex_unlock (&mutexListaNiveles);
	return (planner!=NULL?planner->estado:(t_estado)NULL);
}

t_planificador* cambiarEstadoNivelaFinalizado (char* nivel) {
	pthread_mutex_lock (&mutexListaNiveles);
	t_planificador *planner = NULL;
	planner = dictionary_get(listaNiveles, nivel);
	planner->estado = FINALIZADO;
	pthread_mutex_unlock (&mutexListaNiveles);
	return planner;
}

t_planificador* cambiarEstadoNivelaCorriendo (char* nivel) {
	pthread_mutex_lock (&mutexListaNiveles);
	t_planificador *planner = NULL;
	planner = dictionary_get(listaNiveles, nivel);
	planner->estado = CORRIENDO;
	pthread_mutex_unlock (&mutexListaNiveles);
	return planner;
}

void agregarAListaNiveles(t_planificador* planner) {

	pthread_mutex_lock (&mutexListaNiveles);
	dictionary_put(listaNiveles, planner->nivel.nombre, planner);
	pthread_mutex_unlock (&mutexListaNiveles);
}

t_planificador* quitarDeListaNiveles(char *nivel) {
	pthread_mutex_lock (&mutexListaNiveles);
	t_planificador* planner;
	planner = dictionary_remove(listaNiveles, nivel);
	pthread_mutex_unlock (&mutexListaNiveles);
	return planner;
}

int eliminarNivelesFinalizados () {

	pthread_mutex_lock (&mutexListaNiveles);
	int tamanio=0;
	if (dictionary_size(listaNiveles)>0) {
		t_dictionary *aux = dictionary_create();

		void _buscar_no_finalizados(char *key, t_planificador *planner) {
			if(planner->estado != FINALIZADO)
				dictionary_put(aux, key, planner);
			else
				destruirPlanificador(planner);
		}
		dictionary_iterator(listaNiveles, (void*)_buscar_no_finalizados);
		dictionary_clean(listaNiveles);
		void _agregar(char *key, t_planificador *planner){
			dictionary_put(listaNiveles, key, planner);
		}
		dictionary_iterator(aux, (void*)_agregar);
		dictionary_destroy(aux);
	}

	tamanio = dictionary_size(listaNiveles);
	pthread_mutex_unlock (&mutexListaNiveles);

	return tamanio;
}


void imprimirPersonajePlat (t_personaje* personaje) {
	if(personaje != NULL)
		imprimirPersonaje(personaje, LOGGER);
	else
		log_error(LOGGER, "\n\nimprimirPersonajePlat: ERROR al imprimir personaje NULL!!\n");
}

void imprimirNivelPlat(char *key, t_nivel *nivel) {
	if (nivel != NULL) {
		// llamar funcion imprimir del tad nivel
		imprimirNivel(nivel, LOGGER);
	} else
		log_error(LOGGER, "\n\nimprimirNivelPlat: ERROR al imprimir nivel NULL!!\n");
}

void imprimirListaPersonajesNuevos () {
	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	log_info(LOGGER, "\n\n-- LISTADO Personajes Nuevos en Plataforma: ---\n*************************************************");
	list_iterate(listaPersonajesNuevos, (void*)imprimirPersonajePlat);
	log_info(LOGGER, "\r-- FIN Listado Personajes Nuevos en Plataforma (total: %d) ---\n", list_size(listaPersonajesNuevos));
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);
}

void imprimirListaPersonajesEnJuego () {
	pthread_mutex_lock (&mutexListaPersonajesEnJuego);
	log_info(LOGGER, "\n\n------ LISTADO Personajes En Juego en Plataforma: ---\n*************************************************");
	list_iterate(listaPersonajesEnJuego, (void*)imprimirPersonajePlat);
	log_info(LOGGER, "\r------ FIN Listado Personajes En Juego en Plataforma (total: %d) ---\n", list_size(listaPersonajesEnJuego));
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);
}

void imprimirListaPersonajesFinalizados () {
	pthread_mutex_lock (&mutexListaPersonajesFinalizados);
	log_info(LOGGER, "\n\n--------- LISTADO Personajes Finalizados en Plataforma: ---\n*************************************************");
	list_iterate(listaPersonajesFinalizados, (void*)imprimirPersonajePlat);
	log_info(LOGGER, "\r--------- FIN Listado Personajes Finalizados en Plataforma (total: %d) ---\n", list_size(listaPersonajesFinalizados));
	pthread_mutex_unlock (&mutexListaPersonajesFinalizados);
}

void imprimirListaPersonajesFinAnormal () {
	pthread_mutex_lock (&mutexListaPersonajesFinAnormal);
	log_info(LOGGER, "\n\n--------- LISTADO Personajes Finalizados ANORMALMENTE en Plataforma: ---\n*************************************************");
	list_iterate(listaPersonajesFinAnormal, (void*)imprimirPersonajePlat);
	log_info(LOGGER, "\r--------- FIN Listado Personajes Finalizados ANORMALMENTE en Plataforma (total: %d) ---\n", list_size(listaPersonajesFinAnormal));
	pthread_mutex_unlock (&mutexListaPersonajesFinAnormal);
}


void imprimirListadoNiveles(){
	pthread_mutex_lock(&mutexListaNiveles);
	log_info(LOGGER, "\n\n--------- LISTADO de NIVELES en Plataforma: ---\n*************************************************");
	dictionary_iterator(listaNiveles, (void*)imprimirNivelPlat);
	log_info(LOGGER, "\n\n--------- FIN LISTADO de NIVELES en Plataforma: ---\n*************************************************");
	pthread_mutex_unlock(&mutexListaNiveles);
}


