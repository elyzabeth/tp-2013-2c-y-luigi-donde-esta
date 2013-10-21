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
}

t_personaje* quitarPersonajeNuevoxNivel(char* nivel) {

	t_personaje *personaje;
	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	bool _buscar_x_nivel(t_personaje *p) {
		return !strcasecmp(p->nivel, nivel);
	}
	personaje = list_remove_by_condition(listaPersonajesNuevos, (void*)_buscar_x_nivel);
	//plataforma.personajes_en_juego++;
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);
	return personaje;
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
			return !strcasecmp(p->nivel, nivel);
	}

	pthread_mutex_lock (&mutexListaPersonajesEnJuego);
	aux = list_filter(listaPersonajesEnJuego, (void*)_buscar_x_nivel);
	list_remove_by_condition(listaPersonajesEnJuego, (void*)_buscar_x_nivel);
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);

	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	aux2 = list_filter(listaPersonajesNuevos, (void*)_buscar_x_nivel);
	list_remove_by_condition(listaPersonajesNuevos, (void*)_buscar_x_nivel);
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);

	pthread_mutex_lock (&mutexListaPersonajesFinAnormal);
	list_add_all(listaPersonajesFinAnormal, aux);
	list_add_all(listaPersonajesFinAnormal, aux2);
	pthread_mutex_unlock (&mutexListaPersonajesFinAnormal);

	list_destroy(aux);
	list_destroy(aux2);
}

bool existeNivel(char* nivel) {
	bool existe;
	pthread_mutex_lock (&mutexListaNiveles);
	existe = dictionary_has_key(listaNiveles, nivel);
	pthread_mutex_unlock (&mutexListaNiveles);
	return existe;
}

t_planificador* obtenerNivel(char* nivel) {
	t_planificador *planner = NULL;
	pthread_mutex_lock (&mutexListaNiveles);
	planner = dictionary_get(listaNiveles, nivel);
	pthread_mutex_unlock (&mutexListaNiveles);
	return planner;
}

t_estado obtenerEstadoNivel(char* nivel) {
	t_planificador *planner = NULL;
	pthread_mutex_lock (&mutexListaNiveles);
	planner = dictionary_get(listaNiveles, nivel);
	pthread_mutex_unlock (&mutexListaNiveles);
	return (planner!=NULL?planner->estado:(t_estado)NULL);
}

t_planificador* cambiarEstadoNivelaFinalizado (char* nivel) {
	t_planificador *planner = NULL;
	pthread_mutex_lock (&mutexListaNiveles);
	planner = dictionary_get(listaNiveles, nivel);
	planner->estado = FINALIZADO;
	pthread_mutex_unlock (&mutexListaNiveles);
	return planner;
}

t_planificador* cambiarEstadoNivelaCorriendo (char* nivel) {
	t_planificador *planner = NULL;
	pthread_mutex_lock (&mutexListaNiveles);
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
	t_planificador* planner;
	pthread_mutex_lock (&mutexListaNiveles);
	planner = dictionary_remove(listaNiveles, nivel);
	pthread_mutex_unlock (&mutexListaNiveles);
	return planner;
}

int eliminarNivelesFinalizados () {

	pthread_mutex_lock (&mutexListaNiveles);

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

	pthread_mutex_unlock (&mutexListaNiveles);

	return dictionary_size(listaNiveles);
}


void imprimirPersonaje (t_personaje* personaje) {
	log_info(LOGGER, "Personaje: '%s' - simbolo: '%c' - socket: '%d'", personaje->nombre, personaje->id, personaje->fd);
}


void imprimirListaPersonajesNuevos () {
	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	list_iterate(listaPersonajesNuevos, (void*)imprimirPersonaje);
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);
}

void imprimirListaPersonajesEnJuego () {
	pthread_mutex_lock (&mutexListaPersonajesEnJuego);
	list_iterate(listaPersonajesEnJuego, (void*)imprimirPersonaje);
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);
}
