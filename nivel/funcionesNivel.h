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
#include <pthread.h>

#include "tad_items.h"
#include "commons/log.h"
//#include "commons/collections/list.h"
#include "commons/comunicacion.h"

#include "config/configNivel.h"
#define MAXCANTENEMIGOS 50

//typedef struct enemigo {
//
//} t_enemigo;

t_log* LOGGER;
char NOMBRENIVEL[20+1];
t_list* GUIITEMS;
int MAXROWS, MAXCOLS;

pthread_mutex_t mutexLockGlobalGUI;

pthread_t idHiloInterbloqueo;
pthread_t idHiloEnemigo[MAXCANTENEMIGOS];
// Para comunicacion con hilos;
int32_t fdPipeMainToEnemy [2];

int correrTest();
void principal ();

void inicializarNivel ();
void finalizarNivel ();

void simulacroJuego ();
void ejemploGui ();

// funciones GUI sincronizadas por semaforo mutex
void gui_dibujar();
void gui_mover_personaje (char id, int x, int y);
void gui_crearEnemigo(char id, int x, int y);
void gui_crearCaja(char id, int x, int y, int instancias);


//hilos
void* interbloqueo(void *parametro);
void* enemigo(int *idEnemigo);

// señales
void signal_callback_handler(int signum);

//comunicacion
int enviarMSJNuevoNivel(int sock);


void rnd(int *x, int max);

#endif /* FUNCIONESNIVEL_H_ */
