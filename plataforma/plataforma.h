
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

typedef struct config_plataforma_s
{
  //char id[LARGOID];
  char nombre [10];
  char cantidad_conexiones; /*igual a cantidad de personajes?????*/
  char personajes_en_juego;

} config_plataforma_t;

t_log* LOGGER;
int32_t PUERTO;
config_plataforma_t plataforma;

t_queue *listaPersonajesNuevos;
t_dictionary *listaNiveles;

pthread_mutex_t mutexListaPersonajesNuevos;

void inicializarPlataforma();
void finalizarPlataforma();

void* orquestador(void *parametro);
void nuevoPersonaje(int fdPersonaje);
void nuevoNivel(int fdNivel, header_t header) ;

// señales
void plat_signal_callback_handler(int signum);

//Listas compartidas
void agregarPersonajeNuevo(t_personaje* personaje);

#endif /* plataforma_H_ */



