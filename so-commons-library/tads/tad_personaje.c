/*
 * tad_personaje.c
 *
 *  Created on: 07/10/2013
 *      Author: elyzabeth
 */

#include "tad_personaje.h"

t_personaje* crearPersonaje (char nombre[MAXLENNOMBRE+1], char id, int32_t posX, int32_t posY, int32_t fd, char* nivel) {

	t_personaje* nuevoPersonaje;

	nuevoPersonaje = (t_personaje*)calloc(1, sizeof(t_personaje));
	strcpy(nuevoPersonaje->nombre, nombre);
	nuevoPersonaje->id = id;
	nuevoPersonaje->posActual.x = posX;
	nuevoPersonaje->posActual.y = posY;
	nuevoPersonaje->fd = fd;
	strcpy(nuevoPersonaje->nivel, nivel);
	nuevoPersonaje->recurso = '\0';
	nuevoPersonaje->criterio = 0;
	nuevoPersonaje->rd = 0;

	return nuevoPersonaje;
}

t_personaje* crearPersonajeDesdePersonaje (t_personaje personaje) {

	t_personaje* nuevoPersonaje;

	nuevoPersonaje = (t_personaje*)calloc(1, sizeof(t_personaje));

	strcpy(nuevoPersonaje->nombre, personaje.nombre);
	strcpy(nuevoPersonaje->nivel, personaje.nivel);
	nuevoPersonaje->id = personaje.id;
	nuevoPersonaje->posActual = personaje.posActual;
	nuevoPersonaje->fd = personaje.fd;
	nuevoPersonaje->recurso = personaje.recurso;
	nuevoPersonaje->criterio = personaje.criterio;
	nuevoPersonaje->rd = personaje.rd;

	return nuevoPersonaje;
}

t_personaje* crearPersonajeVacio () {

	t_personaje* nuevoPersonaje;

	nuevoPersonaje = (t_personaje*)calloc(1, sizeof(t_personaje));

//	nuevoPersonaje = (t_personaje*)malloc(sizeof(t_personaje));
//	memset(nuevoPersonaje->nombre, '\0', MAXLENNOMBRE+1);
//	memset(nuevoPersonaje->nivel, '\0', MAXLENNOMBRE+1);
//	nuevoPersonaje->id = 0;
//	nuevoPersonaje->posActual.x = 0;
//	nuevoPersonaje->posActual.y = 0;
//	nuevoPersonaje->fd = 0;
//	nuevoPersonaje->recurso = '\0';

	return nuevoPersonaje;
}

void initPersonje(t_personaje *personaje) {
	memset(personaje, '\0', sizeof(t_personaje));
}

void reiniciarPersonje(t_personaje *personaje) {

	personaje->posActual.x = 0;
	personaje->posActual.y = 0;
	personaje->fd = 0;
	personaje->recurso = '-';
	personaje->criterio = 0;
	personaje->rd = 0;
}

/**
 * @NAME: destruirPersonaje
 * @DESC: libera el espacio resservado para el personaje
 */
void destruirPersonaje (t_personaje * personaje){
	free(personaje);
}

void imprimirPersonaje (t_personaje* p, t_log *LOGGER) {
	log_info(LOGGER, "\r- %s -> Personaje: '%s' ('%c') - recurso: '%c' - posicion: (%d, %d) - socket: '%d' - ", p->nivel, p->nombre, p->id, p->recurso, p->posActual.x,p->posActual.y, p->fd);
}


