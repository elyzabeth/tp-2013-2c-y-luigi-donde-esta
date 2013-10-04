/*
 * personaje.h
 *
 *  Created on: Sep 24, 2013
 *      Author: elizabeth
 */

#ifndef PERSONAJE_H_
#define PERSONAJE_H_

#include <stdio.h>

#include "config/configPersonaje.h"
#include "commons/log.h"
#include "commons/comunicacion.h"
#include "commons/funciones_comunes.h"

t_log* LOGGER;

typedef struct personaje_s
{
  //char id[LARGOID];
  char nombre [25];
  char ip [15+1];
  int puerto;
  char ip_orquestador;
  int puerto_orquestador;

}personaje_t;


void inicializarPersonaje();
void finalizarPersonaje();


#endif /* PERSONAJE_H_ */
