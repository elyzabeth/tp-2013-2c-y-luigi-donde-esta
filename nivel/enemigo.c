/*
 * enemigo.c
 *
 *  Created on: Oct 11, 2013
 *      Author: elizabeth
 */

#include "funcionesNivel.h"

t_posicion buscarPJcercano(t_personaje* listaPJS,t_hiloEnemigo* hiloEnemigo);
void moverEnemigo(t_personaje* listaPJS,t_hiloEnemigo* strTipoEnemigo);//,int32_t x,int32_t y);
void moverEnemigoPorEje(t_personaje* listaPJS,t_hiloEnemigo* id,int32_t x,int32_t y);

void movimientoL(int32_t x,int32_t y,int32_t newx,int32_t newy);
int32_t	validarPosicionEnemigo(int32_t newx,int32_t newy);

void* enemigo (t_hiloEnemigo *enemy) {

	int32_t id = (int32_t) enemy->enemigo.id;
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
		//timeout.tv_sec = sleepEnemigos * 0.001; /// retardo en segundos timeout
		timeout.tv_sec = 0; /// retardo en segundos timeout
		timeout.tv_usec = sleepEnemigos * 1000; //retardo en microsegundos timeout

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



//SECCION de FUNCIONES PARA EL MOVIIENTO DE LOS ENEMIGOS
void moverEnemigo(t_personaje* listaPJS,t_hiloEnemigo* hiloEnemigo)//, x, y)
{
	int32_t x=0, y=0, newx=0, newy =0;
	t_posicion posicionPJ;

	// hay personajes en el nivel?
	if (listaPJS != NULL) {
		posicionPJ = buscarPJcercano(listaPJS, hiloEnemigo);
		x = posicionPJ.x;
		y = posicionPJ.y;
		moverEnemigoPorEje(listaPJS,hiloEnemigo, x, y);
	}

	else{ //No hay personajes en el nivel
		int posValida=0;
		while (!posValida){
			movimientoL(x, y,newx,newy);
			validarPosicionEnemigo(newx,newy);
		}
	}
}
void moverEnemigoPorEje (t_personaje* listaPJS,t_hiloEnemigo* hiloEnemigo,int32_t x,int32_t y)
{
	int32_t newx=0, newy=0;
	if (hiloEnemigo->enemigo.moverPorX){
		// moverEnemigoEnX(listaPJS, id, x, y);
		validarPosicionEnemigo(newx,newy);
	}
	else{
		// moverEnemigoEnY(listaPJS, id, x, y);
		validarPosicionEnemigo(newx,newy);
	}
}

t_posicion buscarPJcercano(t_personaje* listaPJS,t_hiloEnemigo* hiloEnemigo) {
	t_posicion pos;
	//whlie (listaPJS!NULL)
	//CALCULAR EN LA LISTA TODAS LAS DISTANCIAS para encontrar al mas cercano
	return pos;
}

int32_t	validarPosicionEnemigo(int32_t newx,int32_t newy) {
	//FALTA desarrollar
	//  que no pase por cajas ni por PJbloqueados ?? no estan sobre las cajas?
	//MAXROWS MAXCOLS
	return 0;
}

void movimientoL(int32_t x,int32_t y, int32_t newx,int32_t newy) {
	/*
	switch
	case (1)
		randomizar
		if(randomizar)posY++; posX=posX+2
	}
	 */
}
