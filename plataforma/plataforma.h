
#ifndef plataforma_H_
#define plataforma_H_

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <pthread.h>
#include <errno.h>

#include "commons/log.h"
#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "commons/comunicacion.h"
#include "commons/funciones_comunes.h"
#include "config/configPlataforma.h"
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
t_list *listaPersonajesFinalizados;
t_dictionary *listaNiveles;

pthread_mutex_t mutexListaPersonajesNuevos;
pthread_mutex_t mutexListaPersonajesEnJuego;
pthread_mutex_t mutexListaPersonajesFinAnormal;
pthread_mutex_t mutexListaPersonajesFinalizados;
pthread_mutex_t mutexListaNiveles;


// funciones de plataforma
void inicializarPlataforma();
void finalizarPlataforma();

void principal();
void matarHilos();

// señales
void plat_signal_callback_handler(int signum);

// hilos
void* orquestador(t_hiloOrquestador *hiloOrquestador);
void* planificador(t_planificador *nivel);

int enviarMsjAOrquestador (char msj);
int enviarMsjAPlanificador (t_planificador *planner, char msj);

//Listas compartidas
void agregarPersonajeNuevo(t_personaje* personaje);
void agregarPersonajeEnJuego(t_personaje* personaje);
void agregarPersonajeFinalizado(t_personaje* personaje);
void agregarPersonajeFinAnormal(t_personaje* personaje);
_Bool existePersonajexFDEnFinalizados(int fdPersonaje);
t_personaje* quitarPersonajeNuevoxNivel(char* nivel);
t_personaje* quitarPersonajeNuevoxFD(int32_t fdPersonaje);
t_personaje* quitarPersonajeNuevoxNivelxId (char* nivel, char idPersonaje);
t_personaje* quitarPersonajeEnJuegoxFD(int32_t fdPersonaje);
t_personaje* quitarPersonajeEnJuegoxNivelxId (char* nivel, char idPersonaje);
t_personaje* quitarPersonajeFinalizadoxNivelxId (char* nivel, char idPersonaje);
//t_personaje* quitarPersonajexFD(int32_t fdPersonaje);

t_personaje* moverPersonajexFDAFinAnormal(int32_t fdPersonaje);
void moverPersonajeAFinAnormal (char idPersonaje, char *nivel);
void moverPersonajesAFinAnormalxNivel (char *nivel);
void moverPersonajeAFinalizados(char idPersonaje, char *nivel);

bool existeNivel(char* nivel);
t_planificador* obtenerNivel(char* nivel);
t_estado obtenerEstadoNivel(char* nivel);
void agregarAListaNiveles(t_planificador* planner);
int eliminarNivelesFinalizados ();
t_planificador* quitarDeListaNiveles(char *nivel);
t_planificador* cambiarEstadoNivelaFinalizado (char* nivel);
t_planificador* cambiarEstadoNivelaCorriendo (char* nivel);
void imprimirPersonajePlat (t_personaje* personaje);
void imprimirNivelPlat(char *key, t_nivel *nivel);
void imprimirListaPersonajesNuevos();
void imprimirListaPersonajesEnJuego();
void imprimirListaPersonajesFinalizados();
void imprimirListaPersonajesFinAnormal ();
void imprimirListadoNiveles();



#endif /* plataforma_H_ */



