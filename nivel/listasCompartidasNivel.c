/*
 * listasCompartidasNivel.c
 *
 *  Created on: Nov 13, 2013
 *      Author: elizabeth
 */

#include "funcionesNivel.h"


// FUNCIONES de la interfaz grafica sincronizadas con semaforo mutex.
// Porque se acceden concurrentemente desde varios hilos.
void gui_borrarItem(char id) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	ITEM_NIVEL* itm;
    bool _search_by_id(ITEM_NIVEL* item) {
        return item->id == id;
    }
    itm = list_remove_by_condition(GUIITEMS, (void*) _search_by_id);
    if (itm != NULL) free(itm);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_crearPersonaje(char id, int x, int y) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	CrearPersonaje(GUIITEMS, id, x, y);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_crearCaja(char id, int x, int y, int instancias) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	CrearCaja(GUIITEMS, id, x, y, instancias);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_crearEnemigo(char id, int x, int y) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	CrearEnemigo(GUIITEMS, id, x, y);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_moverPersonaje (char id, int x, int y) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	MoverPersonaje(GUIITEMS, id, x, y );
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_restarRecurso (char id) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	restarRecurso(GUIITEMS, id );
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_sumarRecurso (char id) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	sumarRecurso(GUIITEMS, id );
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_dibujar() {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	nivel_gui_dibujar(GUIITEMS, NOMBRENIVEL);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}


void gui_dibujarEnemigo(char * msj) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	nivel_gui_dibujar(GUIITEMS, msj);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_dibujarMsj(char * msj) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	nivel_gui_dibujar(GUIITEMS, msj);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

// Funciones sincronizadas para acceder a listas compartidas
// **********************************************************

void agregarPersonajeEnNivel(t_personaje *personaje) {
	pthread_mutex_lock (&mutexListaPersonajesEnNivel);
	queue_push(listaPersonajesEnNivel, personaje);
	pthread_mutex_unlock (&mutexListaPersonajesEnNivel);
}

void agregarPersonajeMuertoxRecovery(t_personaje *personaje) {
	pthread_mutex_lock (&mutexListaPersonajesMuertosxRecovery);
	queue_push(listaPersonajesMuertosxRecovery, personaje);
	pthread_mutex_unlock (&mutexListaPersonajesMuertosxRecovery);
}

void agregarPersonajeMuertoxEnemigo(t_personaje *personaje) {
	pthread_mutex_lock (&mutexListaPersonajesMuertosxEnemigo);
	queue_push(listaPersonajesMuertosxEnemigo, personaje);
	pthread_mutex_unlock (&mutexListaPersonajesMuertosxEnemigo);
}

void agregarPersonajeEnJuegoNivel(t_personaje *personaje) {
	pthread_mutex_lock (&mutexListaPersonajesJugando);
	list_add(listaPersonajesEnJuego, personaje);
	pthread_mutex_unlock (&mutexListaPersonajesJugando);
}

void agregarPersonajeABloqueadosNivel(t_personaje *personaje) {
	pthread_mutex_lock (&mutexListaPersonajesBloqueados);
	list_add(listaPersonajesBloqueados, personaje);
	pthread_mutex_unlock (&mutexListaPersonajesBloqueados);
}

void agregarPersonajeAFinalizadosNivel(t_personaje *personaje) {
	pthread_mutex_lock (&mutexListaPersonajesFinalizados);
	list_add(listaPersonajesFinalizados, personaje);
	pthread_mutex_unlock (&mutexListaPersonajesFinalizados);
}

void agregarRecursoxPersonaje(t_personaje *personaje, t_vecRecursos *vec) {
	pthread_mutex_lock (&mutexRecursosxPersonajes);
	char idPersonaje[2] = {0};
	idPersonaje[0] = personaje->id;
	dictionary_put(recursosxPersonajes, idPersonaje, vec);
	pthread_mutex_unlock (&mutexRecursosxPersonajes);
}

t_vecRecursos* obtenerRecursoxPersonaje(t_personaje *personaje) {
	pthread_mutex_lock (&mutexRecursosxPersonajes);
	t_vecRecursos *vec;
	char idPersonaje[2] = {0};
	idPersonaje[0] = personaje->id;
	vec = dictionary_get(recursosxPersonajes, idPersonaje);
	pthread_mutex_unlock (&mutexRecursosxPersonajes);
	return vec;
}

void incrementarRecursoxPersonaje(t_personaje *personaje, char idRecurso) {
	pthread_mutex_lock (&mutexRecursosxPersonajes);
	t_vecRecursos *vec;
	char idPersonaje[2] = {0};
	idPersonaje[0] = personaje->id;
	vec = dictionary_get(recursosxPersonajes, idPersonaje);
	vec->recurso[vec->total++] = idRecurso;
	pthread_mutex_unlock (&mutexRecursosxPersonajes);
}

t_vecRecursos* removerRecursoxPersonaje(t_personaje *personaje) {
	pthread_mutex_lock (&mutexRecursosxPersonajes);
	t_vecRecursos *vec;
	char idPersonaje[2] = {0};
	idPersonaje[0] = personaje->id;
	vec = dictionary_remove(recursosxPersonajes, idPersonaje);
	pthread_mutex_unlock (&mutexRecursosxPersonajes);
	return vec;
}

t_caja* obtenerRecurso(char simboloRecurso) {
	pthread_mutex_lock (&mutexListaRecursos);
	t_caja *caja = NULL;
	char simbolo[2] = {0};
	simbolo[0] = simboloRecurso;
	caja = dictionary_get(listaRecursos, simbolo);
	pthread_mutex_unlock (&mutexListaRecursos);
	return caja;
}

int32_t obtenerCantPersonajesEnJuego() {
	pthread_mutex_lock (&mutexListaPersonajesJugando);
	int cant=0;
	cant = list_size(listaPersonajesEnJuego);
	pthread_mutex_unlock (&mutexListaPersonajesJugando);
	return cant;
}

int32_t obtenerCantPersonajesBloqueados() {
	pthread_mutex_lock (&mutexListaPersonajesBloqueados);
	int cant=0;
	cant = list_size(listaPersonajesBloqueados);
	pthread_mutex_unlock (&mutexListaPersonajesBloqueados);
	return cant;
}

int32_t obtenerCantPersonajesEnNivel() {
	pthread_mutex_lock (&mutexListaPersonajesEnNivel);
	int cant=0;
	cant = queue_size(listaPersonajesEnNivel);
	pthread_mutex_unlock (&mutexListaPersonajesEnNivel);
	return cant;
}


t_personaje* quitarPersonajeMuertoxRecovery() {
	pthread_mutex_lock (&mutexListaPersonajesMuertosxRecovery);
	t_personaje *personaje = NULL;
	personaje = queue_pop(listaPersonajesMuertosxRecovery);
	pthread_mutex_unlock (&mutexListaPersonajesMuertosxRecovery);
	return personaje;
}

t_personaje* quitarPersonajeMuertoxEnemigo() {
	pthread_mutex_lock (&mutexListaPersonajesMuertosxEnemigo);
	t_personaje *personaje = NULL;
	personaje = queue_pop(listaPersonajesMuertosxEnemigo);
	pthread_mutex_unlock (&mutexListaPersonajesMuertosxEnemigo);
	return personaje;
}


t_personaje* quitarPersonajeEnNivel(char simboloPersonaje) {
	pthread_mutex_lock (&mutexListaPersonajesEnNivel);
	t_personaje *personaje=NULL, *aux=NULL;
	int i, cant = queue_size(listaPersonajesEnNivel);

	for (i = 0; i < cant; i++ ) {
		aux = queue_pop(listaPersonajesEnNivel);
		if (aux->id != simboloPersonaje) {
			queue_push(listaPersonajesEnNivel, aux);
		} else {
			personaje = aux;
		}
	}
	pthread_mutex_unlock (&mutexListaPersonajesEnNivel);

	return personaje;
}

t_personaje* quitarPersonajeEnJuegoNivel(char simboloPersonaje) {
	pthread_mutex_lock (&mutexListaPersonajesJugando);
	t_personaje *personaje = NULL;
	bool _remove_x_id (t_personaje *p) {
		return (p->id == simboloPersonaje);
	}
	personaje = list_remove_by_condition(listaPersonajesEnJuego, (void*)_remove_x_id);
	pthread_mutex_unlock (&mutexListaPersonajesJugando);

	return personaje;
}

t_personaje* quitarPersonajeBloqueadosNivel(char simboloPersonaje) {
	pthread_mutex_lock (&mutexListaPersonajesBloqueados);
	t_personaje *personaje = NULL;
	bool _remove_x_id (t_personaje *p) {
		return (p->id == simboloPersonaje);
	}

	personaje = list_remove_by_condition(listaPersonajesBloqueados, (void*)_remove_x_id);
	pthread_mutex_unlock (&mutexListaPersonajesBloqueados);

	return personaje;
}

t_personaje* actualizarPosicionPJEnjuego(char idPersonaje, t_posicion posicion) {

	pthread_mutex_lock (&mutexListaPersonajesJugando);
	t_personaje *personaje = NULL;
	bool _busca_x_id (t_personaje *p) {
		return (p->id == idPersonaje);
	}
	personaje = list_find(listaPersonajesEnJuego, (void*)_busca_x_id);

	if (personaje != NULL)
		personaje->posActual = posicion;

	pthread_mutex_unlock (&mutexListaPersonajesJugando);

	return personaje;
}

t_queue* clonarListaPersonajesEnNivel() {

	pthread_mutex_lock (&mutexListaPersonajesEnNivel);
	t_queue *clon = queue_create();
	t_personaje *aux, *personaje;
	int i, cant = queue_size(listaPersonajesEnNivel);

	for (i=0; i<cant; i++) {
		personaje = queue_pop(listaPersonajesEnNivel);
		queue_push(listaPersonajesEnNivel, personaje);

		aux = crearPersonajeDesdePersonaje(*personaje);
		queue_push(clon, aux);
	}

	pthread_mutex_unlock (&mutexListaPersonajesEnNivel);
	return clon;
}


t_list* clonarListaPersonajesEnjuego() {
	pthread_mutex_lock (&mutexListaPersonajesJugando);
	t_list *clon = list_create();
	t_personaje *aux;

	void _add2Clon(t_personaje *personaje) {
		aux = crearPersonajeDesdePersonaje(*personaje);
		list_add(clon, aux);
	}
	list_iterate(listaPersonajesEnJuego, (void*)_add2Clon);

	pthread_mutex_unlock (&mutexListaPersonajesJugando);
	return clon;
}


t_list* clonarListaPersonajesBloqueados() {
	pthread_mutex_lock (&mutexListaPersonajesBloqueados);
	t_list *clon = list_create();
	t_personaje *aux;
	void _add2Clon(t_personaje *personaje) {
		aux = crearPersonajeDesdePersonaje(*personaje);
		list_add(clon, aux);
	}
	list_iterate(listaPersonajesBloqueados, (void*)_add2Clon);
	//list_add_all(clon, listaPersonajesBloq);
	pthread_mutex_unlock (&mutexListaPersonajesBloqueados);
	return clon;
}

t_dictionary* clonarListaRecursosxPersonaje() {
	pthread_mutex_lock (&mutexRecursosxPersonajes);
	t_dictionary *clon = dictionary_create();
	t_vecRecursos *aux;
	void _add2Clon(char *key, t_vecRecursos *vec) {
		aux = crearVecRecursos();
		memcpy(aux->recurso, vec->recurso, sizeof(vec->recurso));
		aux->total = vec->total;
		dictionary_put(clon, key, aux);
	}
	dictionary_iterator(recursosxPersonajes, (void*)_add2Clon);
	//list_add_all(clon, listaPersonajesEnJuego);
	pthread_mutex_unlock (&mutexRecursosxPersonajes);
	return clon;
}

