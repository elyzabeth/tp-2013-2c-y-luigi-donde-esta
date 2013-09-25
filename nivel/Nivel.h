/*
 * nivel.h
 *
 *  Created on: 22/09/2013
 *      Author: elyzabeth
 */

#ifndef NIVEL_H_
#define NIVEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>

#include "commons/log.h"
#include <commons/collections/list.h>

#include "config/configNivel.h"
#include "tad_items.h"


t_log* LOGGER;
char NOMBRENIVEL[20+1];

int correrTest();
void principal ();

void inicializar ();
void finalizar ();

void rnd(int *x, int max);

#endif /* NIVEL_H_ */
