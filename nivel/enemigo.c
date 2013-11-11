/*
* enemigo.c
*
* Created on: Oct 11, 2013
* Author: elizabeth
*/
#include <stdlib.h>
#include "funcionesNivel.h"
#include "tad_enemigo.h"
#include "list.h"
void moverEnemigo(t_hiloEnemigo* hiloEnemigo);
moverEnL(t_hiloEnemigo* hiloEnemigo, posX,posY);
t_posicion moverEnemigoPorEje (t_hiloEnemigo* hiloEnemigo,t_posicion posicionHacia);
t_posicion moverEnemigoEnX(t_hiloEnemigo* hiloEnemigo, t_posicion posicionHacia);
t_posicion moverEnemigoEnY(t_hiloEnemigo* hiloEnemigo, t_posicion posicionHacia);
void estimarMovimientoL(t_hiloEnemigo* hiloEnemigo, int32_t* x,int32_t* x);
t_posicion obternerPersonajeMasCercano(t_posicion miPosicion);
int32_t validarPosicionEnemigo(t_hiloEnemigo* hiloEnemigo, int32_t x,int32_t y);
void* enemigo (t_hiloEnemigo *enemy);
posicionConItem(t_hiloEnemigo* hiloEnemigo, t_posicion posicion);
int32_t validarPosicionEnemigo(t_hiloEnemigo* hiloEnemigo, int32_t X,int32_t Y);


//t_posicion posProhibidas array[]

t_dictionary* listaPosicionesProhibidas = configNivelRecursos();


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
void moverEnemigo(t_hiloEnemigo* hiloEnemigo){
        int32_t* posX, posY;
        *posX=*posY=0;
        int32_t movimientoLfinalizado;
        t_personaje *PJ;
        t_posicion posicionPJ, posicionNueva;
        if (list_size(listaPersonajesEnJuego))/* hay personajes en el nivel?*/
        {

                PJ = obternerPosPersonajeMasCercano(hiloEnemigo->enemigo->posicionActual);
                pthread_mutex_unlock (&mutexListaPersonajesJugando);
                posicionPJ = PJ->posActual;
                posicionNueva = moverEnemigoPorEje(hiloEnemigo, posicionPJ);

                if ((posicionNueva->x == posicionPJ->x)&&(posicionNueva->y == posicionPJ->y)){
                        log_info(LOGGER, "El PJ '%c' ha sido alcanzado por un enemigo:",PJ->id);
                        //NOTIFICAR AL PLANIFICADOR que el personaje perdio una vida
                        // IMPRIMIR por pantalla GUI??
                        }
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
//PARA QUE EL MOVIMIENTO SE REALICE DE A UNO POR VEZ

moverEnL(t_hiloEnemigo* hiloEnemigo, posX,posY){
        while (
			((hiloEnemigo->enemigo->posicionActual->x) != posX) ||
			((hiloEnemigo->enemigo->posicionActual->y) != posY)
			)
        {
                if ((hiloEnemigo->enemigo->posicionActual->x) > posX){
                        (hiloEnemigo->enemigo->posicionActual->x)--;}
                if ((hiloEnemigo->enemigo->posicionActual->x) < posX){
                        (hiloEnemigo->enemigo->posicionActual->x)++;}
                if ((hiloEnemigo->enemigo->posicionActual->y) > posY){
                        (hiloEnemigo->enemigo->posicionActual->y)--;}
                if ((hiloEnemigo->enemigo->posicionActual->y) < posY){
                        (hiloEnemigo->enemigo->posicionActual->y)++;}
        }
}

t_posicion moverEnemigoPorEje (t_hiloEnemigo* hiloEnemigo,t_posicion posicionHacia){
        t_posicion posicionNueva;
        if ((hiloEnemigo->enemigo->moverPorX) &&
        	(hiloEnemigo->enemigo->posicionActual->x != posicionHacia.x)){
        		posicionNueva moverEnemigoEnX(hiloEnemigo, posicionHacia);
        }
        else{
                if(hiloEnemigo->enemigo->posicionActual->y != posicionHacia.y){
                	posicionNueva moverEnemigoEnY(hiloEnemigo, posicionHacia);
                }
        }
        return posicionNueva;
}
t_posicion moverEnemigoEnX(t_hiloEnemigo* hiloEnemigo, t_posicion posicionHacia){
        t_posicion posicionNueva;
        posicionNueva = (hiloEnemigo->enemigo->posicionActual);
        if ((posicionHacia->x) > (hiloEnemigo->enemigo->posicionActual->x)){
                (posicionNueva.x)++;
                if (validarPosicionEnemigo(posicionNueva)){
                        (hiloEnemigo->enemigo->posicionActual->x)++;
                }

        }else{
                (posicionNueva.x)--;
                if (validarPosicionEnemigo(posicionNueva)){
                                (hiloEnemigo->enemigo->posicionActual->x)--;
                        }
        }
        hiloEnemigo->enemigo->moverPorX = 0;
        return posicionNueva;
}
t_posicion moverEnemigoEnY(t_hiloEnemigo* hiloEnemigo, t_posicion posicionHacia){
        t_posicion posicionNueva;
        posicionNueva = (hiloEnemigo->enemigo->posicionActual);
        if ((posicionHacia->y) > (hiloEnemigo->enemigo->posicionActual->y)){
                (posicionNueva.y)++;
                if (validarPosicionEnemigo(posicionNueva)){
                        (hiloEnemigo->enemigo->posicionActual->y)++;
                }

        }else{
                (posicionNueva.y)--;
                if (validarPosicionEnemigo(posicionNueva)){
                                (hiloEnemigo->enemigo->posicionActual->y)--;
                        }
        }
        hiloEnemigo->enemigo->moverPorX = 1;
        return posicionNueva;
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

t_personaje obternerPersonajeMasCercano(t_posicion miPosicion) {
		pthread_mutex_lock (&mutexListaPersonajesJugando);
		t_personaje *pjcercano;
        pjcercano = NULL;
		int32_t cant=0;
        cant = list_size(listaPersonajesEnJuego);
        t_posicion posMasCercana;
        posMasCercana.x = 1000;
        posMasCercana.y = 1000;
        int32_t distanciaMasCercana = calcularDistanciaCoord(miPosicion, posMasCercana);
        	void buscarPJcercano(t_personaje *p) {
					int32_t distancia = calcularDistanciaCoord(miPosicion, p->posActual);
							if (distanciaMasCercana > distancia) {
									posMasCercana = p->posActual;
									distanciaMasCercana = distancia;
									pjcercano = p;
							}
			}
        if(cant > 0){
                list_iterate(listaPersonajesEnJuego, (void*)buscarPJcercano);

        }
        pthread_mutex_unlock (&mutexListaPersonajesJugando);
        return pjcercano;
}

posicionConItem(t_hiloEnemigo* hiloEnemigo, t_posicion posicion){
    int32_t hayCaja = 0;
void hayItemEn(t_caja *caja){
			if (posicion->x == caja->POSX && posicion->x == caja->POSY){
			hayCaja = 1;
			}
        }
list_iterate(listaPosicionesProhibidas, (void*)hayItemEn);
        return hayCaja;
}

int32_t        validarPosicionEnemigo(t_hiloEnemigo* hiloEnemigo, int32_t X,int32_t Y) {
        t_posicion pos;
        pos->x=X;pos->y=Y;
        int32_t pAux=posicionConItem(hiloEnemigo,pos);
        if((Y <= MAXROWS) && (X <= MAXCOLS) && pAux){
                if (!posicionConItem(pos)){
                        return 1;//PosicionOK
                }
        }
        return 0;// POSICION INVALIDA
}

