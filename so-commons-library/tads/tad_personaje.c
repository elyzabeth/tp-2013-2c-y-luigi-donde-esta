/*
 * tad_personaje.c
 *
 *  Created on: 07/10/2013
 *      Author: elyzabeth
 */

#include "tad_personaje.h"

t_personaje* crearPersonaje (char nombre[MAXLENNOMBRE+1], char id, int32_t posX, int32_t posY, int32_t fd, char* nivel) {

	t_personaje* nuevoPersonaje;

	nuevoPersonaje = (t_personaje*)malloc(sizeof(t_personaje));
	strcpy(nuevoPersonaje->nombre, nombre);
	nuevoPersonaje->id = id;
	nuevoPersonaje->posActual.x = posX;
	nuevoPersonaje->posActual.y = posY;
	nuevoPersonaje->fd = fd;
	strcpy(nuevoPersonaje->nivel, nivel);

	return nuevoPersonaje;
}

t_personaje* crearPersonajeVacio () {

	t_personaje* nuevoPersonaje;

	nuevoPersonaje = (t_personaje*)malloc(sizeof(t_personaje));
	memset(nuevoPersonaje->nombre, '\0', MAXLENNOMBRE+1);
	memset(nuevoPersonaje->nivel, '\0', MAXLENNOMBRE+1);
	nuevoPersonaje->id = 0;
	nuevoPersonaje->posActual.x = 0;
	nuevoPersonaje->posActual.y = 0;
	nuevoPersonaje->fd = 0;

	return nuevoPersonaje;
}

/**
 * @NAME: destruirPersonaje
 * @DESC: libera el espacio resservado para el personaje
 */
void destruirPersonaje (t_personaje * personaje){
	free(personaje);
}



