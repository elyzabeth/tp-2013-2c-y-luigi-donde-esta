
#ifndef plataforma_H_
#define plataforma_H_

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <pthread.h>

#include "commons/comunicacion.h"
#include "config/configPlataforma.h"
#include "commons/log.h"
#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "tads/tad_personaje.h"
#include "tads/tad_nivel.h"
#include "tads/tad_planificador.h"
#include "tads/tad_caja.h"


typedef struct config_plataforma_s
{
  //char id[LARGOID];
  char nombre [10];
  char cantidad_conexiones; /*igual a cantidad de personajes?????*/
  char personajes_en_juego;

} config_plataforma_t;

typedef struct orq {
	pthread_t tid;
	int32_t fdPipe[2]; // fdPipe[0] de lectura/ fdPipe[1] de escritura
} t_hiloOrquestador;

t_hiloOrquestador hiloOrquestador;

t_log* LOGGER;
int32_t PUERTO;
config_plataforma_t plataforma;

t_list *listaPersonajesNuevos;
t_list *listaPersonajesEnJuego;
t_list *listaPersonajesFinAnormal;
t_dictionary *listaNiveles;

pthread_mutex_t mutexListaPersonajesNuevos;
pthread_mutex_t mutexListaPersonajesEnJuego;
pthread_mutex_t mutexListaPersonajesFinAnormal;
pthread_mutex_t mutexListaNiveles;


void inicializarPlataforma();
void finalizarPlataforma();

void principal();
void matarHilos();

void nuevoPersonaje(int fdPersonaje, fd_set *master, int *max_desc);
void nuevoNivel(int fdNivel, header_t header) ;

// hilos
void* orquestador(t_hiloOrquestador *hiloOrquestador);
void* planificador(t_planificador *nivel);

// señales
void plat_signal_callback_handler(int signum);

//Listas compartidas
void agregarPersonajeNuevo(t_personaje* personaje);
t_personaje* quitarPersonajeNuevoxNivel(char* nivel);
t_personaje* quitarPersonajexFD(int32_t fdPersonaje);
void agregarPersonajeEnJuego(t_personaje* personaje);
void moverPersonajesAFinAnormalxNivel (char *nivel);
bool existeNivel(char* nivel);
t_planificador* obtenerNivel(char* nivel);
t_estado obtenerEstadoNivel(char* nivel);
void agregarAListaNiveles(t_planificador* planner);
int eliminarNivelesFinalizados ();
t_planificador* quitarDeListaNiveles(char *nivel);
t_planificador* cambiarEstadoNivelaFinalizado (char* nivel);
t_planificador* cambiarEstadoNivelaCorriendo (char* nivel);



#endif /* plataforma_H_ */



