/*
 * tad_enemigo.h
 *
 *  Created on: Oct 17, 2013
 *      Author: elizabeth
 */

#ifndef TAD_ENEMIGO_H_
#define TAD_ENEMIGO_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#pragma pack(1)
typedef struct enemy {
	int32_t id;
	pthread_t tid;
	t_enemigo enemigo;
	int32_t fdPipe[2]; // fdPipe[0] de lectura / fdPipe[1] de escritura
} t_hiloEnemigo;
#pragma pack(0)

t_hiloEnemigo* crearEnemigo(int32_t idEnemigo);
void destruirEnemigo (t_hiloEnemigo* enemigo);

//Esctructura enemigo para la logica
typedef struct {
	 int32_t posx;
	 int32_t posy;
} t_posicion;

typedef struct {
	 t_posicion posicionActual;
	 t_posicion posicionAnterior;
	 int32_t moverPorX;//flag para q no se muevan por el mismo eje
 } t_enemigo;

t_posicion buscarPJcercano(t_personaje* listaPJS,t_hiloEnemigo* hiloEnemigo);
void moverEnemigo(t_personaje* listaPJS,t_hiloEnemigo* strTipoEnemigo,int32_t x,int32_t y);
void moverEnemigoPorEje(t_personaje* listaPJS,t_hiloEnemigo* id,int32_t x,int32_t y);

void movimientoL(int32_t x,int32_t y,int32_t newx,int32_t newy);
int32_t	validarPosicionEnemigo(int32_t newx,int32_t newy);

#endif /* TAD_ENEMIGO_H_ */
