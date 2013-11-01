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

t_personaje* quitarPersonajeNuevoxNivel(char* nivel) {

	t_personaje *personaje;
	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	bool _buscar_x_nivel(t_personaje *p) {
		return !strcasecmp(p->nivel, nivel);
	}
	personaje = list_remove_by_condition(listaPersonajesNuevos, (void*)_buscar_x_nivel);
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);
	imprimirListaPersonajesNuevos();
	return personaje;
}

t_personaje* quitarPersonajexFD(int32_t fdPersonaje) {

	t_personaje *personaje;
	bool _buscar_x_fd(t_personaje *p) {
		return (p->fd == fdPersonaje);
	}
	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	personaje = list_remove_by_condition(listaPersonajesNuevos, (void*)_buscar_x_fd);
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);

	pthread_mutex_lock (&mutexListaPersonajesEnJuego);
	personaje = list_remove_by_condition(listaPersonajesEnJuego, (void*)_buscar_x_fd);
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);

	pthread_mutex_lock (&mutexListaPersonajesFinAnormal);
	list_add(listaPersonajesFinAnormal, personaje);
	pthread_mutex_unlock (&mutexListaPersonajesFinAnormal);

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
	imprimirListaPersonajesEnJuego();
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

void moverPersonajeAFinAnormal (char idPersonaje, char *nivel) {
	//TODO!!!
	t_personaje *personaje = NULL;
	t_personaje *personaje2 = NULL;

	bool _buscar_xnivel_xid(t_personaje *p) {
			return (strcasecmp(p->nivel, nivel) && p->id == idPersonaje);
	}

	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	personaje = list_remove_by_condition(listaPersonajesNuevos, (void*)_buscar_xnivel_xid);
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);

	pthread_mutex_lock (&mutexListaPersonajesEnJuego);
	personaje2 = list_remove_by_condition(listaPersonajesEnJuego, (void*)_buscar_xnivel_xid);
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);

	if (personaje != NULL)
		agregarPersonajeFinAnormal(personaje);
	else if (personaje2 != NULL)
		agregarPersonajeFinAnormal(personaje2);

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


void imprimirPersonajePlat (t_personaje* personaje) {
	imprimirPersonaje(personaje, LOGGER);
}


void imprimirListaPersonajesNuevos () {
	pthread_mutex_lock (&mutexListaPersonajesNuevos);
	log_info(LOGGER, "\n\n--- LISTADO Personajes Nuevos en Plataforma: ---\n*************************************************");
	list_iterate(listaPersonajesNuevos, (void*)imprimirPersonajePlat);
	log_info(LOGGER, "\r--- FIN Listado Personajes Nuevos en Plataforma (total: %d) ---\n", list_size(listaPersonajesNuevos));
	pthread_mutex_unlock (&mutexListaPersonajesNuevos);
}

void imprimirListaPersonajesEnJuego () {
	pthread_mutex_lock (&mutexListaPersonajesEnJuego);
	log_info(LOGGER, "\n\n --- LISTADO Personajes En Juego en Plataforma: ---\n*************************************************");
	list_iterate(listaPersonajesEnJuego, (void*)imprimirPersonajePlat);
	log_info(LOGGER, "\r--- FIN Listado Personajes En Juego en Plataforma (total: %d) ---\n", list_size(listaPersonajesEnJuego));
	pthread_mutex_unlock (&mutexListaPersonajesEnJuego);
}
