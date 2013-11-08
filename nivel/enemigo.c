/*
 * enemigo.c
 *
 *  Created on: Oct 11, 2013
 *      Author: elizabeth
 */
#include <stdlib.h>
#include "funcionesNivel.h"
#include "tad_enemigo.h"
#include "list.h"

t_posicion buscarPJcercano(t_personaje* listaPJS,t_hiloEnemigo* hiloEnemigo);
void moverEnemigo(t_personaje* listaPJS,t_hiloEnemigo* strTipoEnemigo);//,int32_t x,int32_t y);
void moverEnemigoPorEje(t_personaje* listaPJS,t_hiloEnemigo* id,int32_t x,int32_t y);

void movimientoL(int32_t x,int32_t y,int32_t newx,int32_t newy);
int32_t	validarPosicionEnemigo(int32_t newx,int32_t newy);

void* enemigo (t_hiloEnemigo *enemy) {

	int32_t id = (int32_t) enemy->id;
	int32_t sleepEnemigos;
	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;
	int x=10, y=15;
	int fin = false;
	struct timeval timeout;

	log_info(LOGGER, "Enemigo '%c' Iniciado.", id);

	// Obtengo parametro del archivo de configuracion
	sleepEnemigos = configNivelSleepEnemigos();

	FD_ZERO(&master);

	// Agrego descriptor del Pipe con Nivel.
	agregar_descriptor(enemy->fdPipe[0], &master, &max_desc);

	rnd(&x, MAXCOLS);
	rnd(&y, MAXROWS);
	gui_crearEnemigo(id, x, y);
	gui_dibujar();

	while (!fin) {

		FD_ZERO (&read_fds);
		read_fds = master;
		timeout.tv_sec = sleepEnemigos * 0.001; /// retardo en segundos timeout
		timeout.tv_usec = 0; //retardo en microsegundos timeout

		ret = select(max_desc+1, &read_fds, NULL, NULL, &timeout);
		if(ret == -1) {
			printf("Enemigo '%c': ERROR en select.", id);
			sleep(1);
			continue;
		}
		if (ret == 0) {
			//log_debug(LOGGER, "Enemigo '%c': timeout", enemy->id);

			//TODO agregar logica del enemigo
			// Cambiar este movimiento aleatorio por el que corresponde
			rnd(&x, MAXCOLS);
			rnd(&y, MAXROWS);

			gui_moverPersonaje(id, x, y );
			gui_dibujar();

		}
		if (ret > 0) {
			for(i = 0; i <= max_desc; i++)
			{

				if (FD_ISSET(i, &read_fds) && (i == enemy->fdPipe[0]))
				{
					header_t header;
					log_info(LOGGER, "Enemigo '%c': Recibo mensaje desde Nivel por Pipe", id);
					read (enemy->fdPipe[0], &header, sizeof(header_t));

					log_debug(LOGGER, "Enemigo '%c': mensaje recibido '%d'", id, header.tipo);
					if (header.tipo == FINALIZAR) {
						log_debug(LOGGER, "Enemigo '%c': '%d' ES FINALIZAR", id, header.tipo);
						fin = true;
						break;
					}

				}
			}
		}

	}

	log_info(LOGGER, "FINALIZANDO ENEMIGO '%c' \n", id);
	gui_borrarItem(id);

	pthread_exit(NULL);
}



//SECCION de FUNCIONES PARA EL MOVIMIENTO DE LOS ENEMIGOS
void moverEnemigo(t_hiloEnemigo* hiloEnemigo)//, x, y)
{
	int32_t* posX, posY;
	*posX=*posY=0;
	t_posicion posicionPJ;
	if (list_size(listaPersonajesEnJuego))/* hay personajes en el nivel?*/
	{
		posicionPJ = obternerPosPersonajeMasCercano(hiloEnemigo->enemigo->posicionActual);
		moverEnemigoPorEje(hiloEnemigo, posicionPJ);
	}

	else{ //No hay personajes en el nivel
		int posValida=0;
		t_posicion posAux;
		if (movimientoLfinalizado){
			while (!posValida){
				estimarMovimientoL(hiloEnemigo, posX, posY);
				posValida = validarPosicionEnemigo(hiloEnemigo, posX, posY);
			}
		}
		moverEnL(hiloEnemigo, posX,posY);
		movimientoLfinalizado = 1;
	}
}
//PARA QUE EL MOVIEMIENTO SE REALICE DE A UNO POR VEZ

moverEnL(t_hiloEnemigo* hiloEnemigo, posX,posY){
	while ((hiloEnemigo->enemigo->posicionActual->x) != posX) OR
		(hiloEnemigo->enemigo->posicionActual->y) != posY){
		if ((hiloEnemigo->enemigo->posicionActual->x) > posX){
			(hiloEnemigo->enemigo->posicionActual->x)--}
		if ((hiloEnemigo->enemigo->posicionActual->x) < posX){
			(hiloEnemigo->enemigo->posicionActual->x)++}
		if ((hiloEnemigo->enemigo->posicionActual->y) > posY){
			(hiloEnemigo->enemigo->posicionActual->y)--}
		if ((hiloEnemigo->enemigo->posicionActual->y) < posY){
			(hiloEnemigo->enemigo->posicionActual->y)++}
	}
}
void moverEnemigoPorEje (t_hiloEnemigo* hiloEnemigo,t_posicion posicionHacia)
{
	t_posicion posicionNueva;
	if (hiloEnemigo->enemigo->moverPorX){
		posicionNueva moverEnemigoEnX(hiloEnemigo, posicionHacia);
	}
	else{
		posicionNueva moverEnemigoEnY(hiloEnemigo, posicionHacia);
		validarPosicionEnemigo(posicionNueva);
	}
}
t_posicion moverEnemigoEnX(t_hiloEnemigo* hiloEnemigo, t_posicion posicionHacia){
	t_posicion posicionNueva;
	posicionNueva = (hiloEnemigo->enemigo->posicionActual);
	if ((posicionHacia->x) > (hiloEnemigo->enemigo->posicionActual->x)){
		(posicionNueva.x)++;
		if (validarPosicionEnemigo(posicionNueva)){
			(hiloEnemigo->enemigo->x)++;
		}

	}else{
		(posicionNueva.x)--;
		if (validarPosicionEnemigo(posicionNueva)){
				(hiloEnemigo->enemigo->x)--;
			}
	}
	hiloEnemigo->enemigo->moverPorX = 0;
	if (posicionNueva == posicionHacia){
		//print("pj alcanzado");
		}
	return posicionNueva;
}
int32_t	validarPosicionEnemigo(t_hiloEnemigo* hiloEnemigo, int32_t* x,int32_t* x) {
	//FALTA desarrollar
	//  que no pase por cajas   usar mutex para la lista de cajas
	//MAXROWS MAXCOLS
	return 1; //PosicionOK
}

void estimarMovimientoL(t_hiloEnemigo* hiloEnemigo, int32_t* x,int32_t* x) {
	int32_t posX,posY;
	posX = hiloEnemigo->enemigo->posicionActual->x;
	posY = hiloEnemigo->enemigo->posicionActual->y;
	int r = rand() % 8;
	switch(r) {
		case 1:
			posY++; posX=posX+2;
		break;
		case 2:
			posY++; posX=posX-2;
		break;
		case 3:
			posY--; posX=posX+2;
		break;
		case 4:
			posY--; posX=posX-2;
		break;
		case 5:
			posY=posY+2; posX++;
		break;
		case 6:
			posY=posY-2; posX++;
		break;
		case 7:
			posY=posY+2; posX--;
		break;
		case 0:
			posY=posY-2; posX--;
		break;
	}
}

void buscarPJcercano(t_personaje *pj) {
	int32_t distancia = calcularDistanciaCoord(miPosicion, pj->posActual);
		if (distanciaMasCercana > distancia) {
			posMasCercana = p->posActual;
			distanciaMasCercana = distancia;
		}
}
t_posicion obternerPosPersonajeMasCercano(t_posicion miPosicion) {
	pthread_mutex_lock (&mutexListaPersonajesJugando);
	int cant=0;
	cant = list_size(listaPersonajesEnJuego);
	t_posicion posMasCercana;
	posMasCercana.x = 1000;
	posMasCercana.y = 1000;
	int32_t distanciaMasCercana = calcularDistanciaCoord(miPosicion, posMasCercana);
	if(cant > 0){
		list_iterate(listaPersonajesEnJuego, (void*)buscarPJcercano);
		pthread_mutex_unlock (&mutexListaPersonajesJugando);
		return posMasCercana;
	}
}
