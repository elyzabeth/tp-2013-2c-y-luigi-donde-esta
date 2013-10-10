/*
 * tad_personaje.c
 *
 *  Created on: 07/10/2013
 *      Author: elyzabeth
 */

#include "tad_personaje.h"

t_personaje* crearPersonaje (char nombre[MAXLENNOMBRE+1], char id, int32_t posX, int32_t posY, int32_t fd) {

	t_personaje* nuevoPersonaje;

	nuevoPersonaje = (t_personaje*)malloc(sizeof(t_personaje));
	strcpy(nuevoPersonaje->nombre, nombre);
	nuevoPersonaje->id = id;
	nuevoPersonaje->posActual.x = posX;
	nuevoPersonaje->posActual.y = posY;

	return nuevoPersonaje;
}

/**
 * @NAME: destruirPersonaje
 * @DESC: libera el espacio resservado para el personaje
 */
void destruirPersonaje (t_personaje * personaje){
	free(personaje);
}



