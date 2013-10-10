/*
 * funcionesNivel.h
 *
 *  Created on: Oct 3, 2013
 *      Author: elyzabeth
 */

#ifndef FUNCIONESNIVEL_H_
#define FUNCIONESNIVEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "tad_items.h"
#include "commons/log.h"
//#include "commons/collections/list.h"
#include "commons/comunicacion.h"

#include "config/configNivel.h"

typedef struct enemigo {

} t_enemigo;

t_log* LOGGER;
char NOMBRENIVEL[20+1];
t_list* GUIITEMS;
int MAXROWS, MAXCOLS;
char *buffer_header;

int correrTest();
void principal ();

void inicializarNivel ();
void finalizarNivel ();

// señales
void signal_callback_handler(int signum);

//comunicacion
int enviarMSJNuevoNivel(int sock);


void rnd(int *x, int max);

#endif /* FUNCIONESNIVEL_H_ */
