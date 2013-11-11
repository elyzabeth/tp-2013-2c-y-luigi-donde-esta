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
#include "commons/funciones_comunes.h"

#include "config/configNivel.h"
#include "tads/tad_nivel.h"
#include "tads/tad_enemigo.h"
#include "tads/tad_caja.h"
#include "tads/tad_personaje.h"

#define EVENT_SIZE ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN ( 1024 * EVENT_SIZE )

#define MAXCANTENEMIGOS 50

int32_t watchDescriptor;
int32_t notifyFD;

t_log* LOGGER;
//char NOMBRENIVEL[20+1];
char NOMBRENIVEL[MAXCHARLEN+1];


int MAXROWS, MAXCOLS;
t_list *GUIITEMS;
t_list *listaPersonajesEnJuego;
t_list *listaPersonajesBloqueados;
t_list *listaPersonajesFinalizados;
t_dictionary *recursosxPersonajes;

// Diccionario de recursos con clave=simbolo data=t_caja
t_dictionary *listaRecursos;
t_list *listaEnemigos;

pthread_mutex_t mutexLockGlobalGUI;
pthread_mutex_t mutexListaPersonajesJugando;
pthread_mutex_t mutexListaPersonajesBloqueados;
pthread_mutex_t mutexListaPersonajesFinalizados;
pthread_mutex_t mutexListaRecursos;
pthread_mutex_t mutexRecursosxPersonajes;

typedef struct {
	pthread_t tid;
	int32_t fdPipe[2];
} t_hiloInterbloqueo;

t_hiloInterbloqueo hiloInterbloqueo;

typedef struct {
	int32_t recurso[100];
	int32_t total;
} t_vecRecursos;

int correrTest();
void principal ();

void inicializarNivel ();
void finalizarNivel ();
void finalizarPersonajeNivel(t_personaje *personaje);
int crearNotifyFD();
t_vecRecursos* crearVecRecursos();
void destruirVecRecursos(t_vecRecursos *vecRecursos);
void agregarRecursoVec(t_vecRecursos *vecRecursos, char recurso);


// funciones GUI sincronizadas por semaforo mutex
void gui_dibujar();
void gui_moverPersonaje (char id, int x, int y);
void gui_restarRecurso (char id);
void gui_crearEnemigo(char id, int x, int y);
void gui_crearCaja(char id, int x, int y, int instancias);
void gui_crearPersonaje(char id, int x, int y);
void gui_borrarItem(char id);

// funciones listas compartidas
int32_t obternerCantPersonajesEnJuego();
void moverPersonajeABloqueados(char simboloPersonaje);
void agregarPersonajeEnJuegoNivel(t_personaje *personaje);
void agregarPersonajeBloqueadosNivel(t_personaje *personaje);
void imprimirPersonajeNivel (t_personaje* personaje);
t_list* clonarListaPersonajesBloqueados();
t_vecRecursos* removerRecursoxPersonaje(t_personaje *personaje);
t_caja* obtenerRecurso(char simboloRecurso);

//hilos
void* interbloqueo(t_hiloInterbloqueo *hiloInterbloqueo);
void* enemigo (t_hiloEnemigo *enemy);

// señales
void signal_callback_handler(int signum);

//comunicacion
int enviarMsjAInterbloqueo (char msj);
int enviarMSJNuevoNivel(int sock);
int enviarMsjCambiosConfiguracion(int sock);
int tratarNuevoPersonaje(int sock, header_t header, fd_set *master);
int tratarSolicitudUbicacion(int sock, header_t header, fd_set *master);
int tratarSolicitudRecurso(int sock, header_t header, fd_set *master);
int tratarMovimientoRealizado(int sock, header_t header, fd_set *master);
int tratarPlanNivelFinalizado(int sock, header_t header, fd_set *master);
int tratarMuertePersonaje(int sock, header_t header, fd_set *master);

void rnd(int *x, int max);

#endif /* FUNCIONESNIVEL_H_ */
