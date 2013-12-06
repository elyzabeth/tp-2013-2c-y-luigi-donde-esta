/*
 * personaje.h
 *
 *  Created on: Sep 24, 2013
 *      Author: elizabeth
 */

#ifndef PERSONAJE_H_
#define PERSONAJE_H_

#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>

#include "config/configPersonaje.h"
#include "commons/log.h"
#include "commons/comunicacion.h"
#include "commons/funciones_comunes.h"
#include "tads/tad_personaje.h"
#include "tads/tad_caja.h"


typedef enum motivo_muerte {
	MUERTE_POR_SIGTERM,
	MUERTE_POR_SIGKILL,
	MUERTE_POR_ENEMIGO,
	MUERTE_POR_INTERBLOQUEO
} MOTIVO_MUERTE;

typedef struct personaje_s
{
	//char id[LARGOID];
	char nombre [25];
	char ip [15+1];
	int puerto;
	char ip_orquestador[15+1];
	int puerto_orquestador;
	int reiniciar;
} personaje_t;


#pragma pack(1)
typedef struct {
	t_personaje personaje;
	t_objetivosxNivel objetivos;
	//t_posicion posicionActual;
	int32_t moverPorX;
	int32_t estado;
	int32_t objetivosConseguidos;
	pthread_t tid;
	int32_t fdPipe[2]; // fdPipe[0] de lectura / fdPipe[1] de escritura
} t_hilo_personaje;
#pragma pack(0)

#pragma pack(1)
typedef struct {
	char simbolo;
	t_posicion posicion;
} t_proximoObjetivo;
#pragma pack(0)

t_log* LOGGER;
int32_t VIDAS;
int32_t REINTENTOS;
char CONFIG_FILE[MAXCHARLEN];

t_queue *planDeNiveles;
t_list *listaHilosxNivel;

pthread_mutex_t mutexEnvioMensaje;
pthread_mutex_t mutexVidas;
pthread_mutex_t mutexListaHilosxNivel;
pthread_mutex_t mutexReinicio;

personaje_t personaje;


// Prototipos de funciones
int principal(int argc, char *argv[]);
void inicializarPersonaje();
void inicializarVariablesGlobales();
void reiniciar(bool valor);
void reinicioNivelCompleto();
void finalizarPersonaje();
void finalizarHilosPersonaje();

void levantarHilosxNivel() ;
void esperarHilosxNivel();
void* personajexNivel (t_hilo_personaje *hiloPxN);
int chequearFinTodosLosNiveles();

t_hilo_personaje* crearEstructuraHiloPersonaje(t_objetivosxNivel *oxn);
void destruirEstructuraHiloPersonaje(t_hilo_personaje* hiloPersonaje);

t_hilo_personaje* quitarHiloPersonajexTid (int32_t tid);

void per_signal_callback_handler(int signum);
void manejoSIGTERM();
void imprimirVidasyReintentos();

int32_t incrementarVida();
int32_t decrementarVida();

int enviarMsjPorPipePJ (int32_t fdPipe, char msj);
int recibirHeaderNuevoMsj (int sock, header_t *header, fd_set *master);
int enviarMsjNuevoPersonaje( int sock, t_hilo_personaje *hiloPxN );
int enviarInfoPersonaje(int sock, t_hilo_personaje *hiloPxN);
int enviarInfoPersonaje2(int sock);
int enviarSolicitudUbicacion (int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN);
int enviarMsjPlanDeNivelFinalizado( int sock , t_hilo_personaje *hiloPxN);
int enviarMsjMuertePersonajePlan ( int sock, t_hilo_personaje *hiloPxN );
int recibirUbicacionRecursoPlanificador( int sock, fd_set *master, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN );
int gestionarTurnoConcedido(int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN);
int gestionarRecursoConcedido (int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN, int *fin);

void enviarMsjPlanDeNivelesConcluido();
void enviarMsjMuerteDePersonajeAlOrq();


#endif /* PERSONAJE_H_ */
