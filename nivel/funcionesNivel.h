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
#include <sys/inotify.h>

#include "tad_items.h"
#include "commons/log.h"
//#include "commons/collections/list.h"
#include "commons/comunicacion.h"

#include "config/configNivel.h"
#include "tads/tad_nivel.h"
#include "tads/tad_enemigo.h"

#define MAXCANTENEMIGOS 50

int32_t watchDescriptor;
int32_t notifyFD;

t_log* LOGGER;
char NOMBRENIVEL[20+1];
t_list* GUIITEMS;
int MAXROWS, MAXCOLS;

t_list *listaEnemigos;
pthread_mutex_t mutexLockGlobalGUI;

pthread_t idHiloInterbloqueo;


int correrTest();
void principal ();

void inicializarNivel ();
void finalizarNivel ();
int crearNotifyFD();

void simulacroJuego ();
void ejemploGui ();

// funciones GUI sincronizadas por semaforo mutex
void gui_dibujar();
void gui_mover_personaje (char id, int x, int y);
void gui_crearEnemigo(char id, int x, int y);
void gui_crearCaja(char id, int x, int y, int instancias);
void gui_crearPersonaje(char id, int x, int y);
void gui_borrarItem(char id);

//hilos
void* interbloqueo(void *parametro);
void* enemigo (t_enemigo *enemy);

// señales
void signal_callback_handler(int signum);

//comunicacion
int enviarMSJNuevoNivel(int sock);


void rnd(int *x, int max);

#endif /* FUNCIONESNIVEL_H_ */
