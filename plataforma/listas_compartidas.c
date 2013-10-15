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
