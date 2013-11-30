/*
 * enemigo.c
 *
 * Created on: Oct 11, 2013
 * Author: elizabeth
 */

//#include <stdlib.h>
//#include "tads/tad_enemigo.h"
//#include <commons/collections/list.h>

#include "funcionesNivel.h"

void moverEnemigo(t_hiloEnemigo* hiloEnemigo);
void moverDeUnoHacia(t_hiloEnemigo* hiloEnemigo,int32_t* posX,int32_t* posY);
t_posicion moverEnemigoPorEje (t_hiloEnemigo* hiloEnemigo,t_posicion posicionHacia);
t_posicion moverEnemigoEnX(t_hiloEnemigo* hiloEnemigo, t_posicion posicionHacia);
t_posicion moverEnemigoEnY(t_hiloEnemigo* hiloEnemigo, t_posicion posicionHacia);
void estimarMovimientoL(t_hiloEnemigo* hiloEnemigo, int32_t* x,int32_t* y);
t_personaje obternerPersonajeMasCercano(t_posicion miPosicion);
int32_t validarPosicionEnemigo(t_hiloEnemigo* hiloEnemigo, int32_t X,int32_t Y);
void* enemigo (t_hiloEnemigo *enemy);
int32_t posicionConItem(t_hiloEnemigo* hiloEnemigo, t_posicion posicion);
t_dictionary *listaPosicionesProhibidas;

void verificarPosicionPersonajes(t_hiloEnemigo *enemy);
//t_posicion posProhibidas array[]

t_dictionary* listaPosicionesProhibidas; // = configNivelRecursos();


void* enemigo (t_hiloEnemigo *enemy) {
	listaPosicionesProhibidas = configNivelRecursos();
	int32_t id = (int32_t) enemy->id;
	int32_t sleepEnemigos;
	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;
	//int x=10, y=15;
	int fin = false;
	struct timeval timeout;

	log_info(LOGGER, "Enemigo '%c' Iniciado.", id);

	// Obtengo parametro del archivo de configuracion
	sleepEnemigos = configNivelSleepEnemigos();

	FD_ZERO(&master);

	// Agrego descriptor del Pipe con Nivel.
	agregar_descriptor(enemy->fdPipe[0], &master, &max_desc);

//	rnd(&x, MAXCOLS);
//	rnd(&y, MAXROWS);
	enemy->enemigo.posicionActual.x = (enemy->tid)%MAXCOLS;
	enemy->enemigo.posicionActual.y = (enemy->tid)%MAXROWS;

	rnd(&(enemy->enemigo.posicionActual.x), MAXCOLS);
	rnd(&(enemy->enemigo.posicionActual.y), MAXROWS);

	enemy->enemigo.posicionEleSiguiente.x = enemy->enemigo.posicionActual.x;
	enemy->enemigo.posicionEleSiguiente.y = enemy->enemigo.posicionActual.y;

	gui_crearEnemigo(id, enemy->enemigo.posicionActual.x, enemy->enemigo.posicionActual.y);
	gui_dibujar();

	while (!fin) {
		FD_ZERO (&read_fds);
		read_fds = master;
		//timeout.tv_sec = sleepEnemigos * 0.001;
		timeout.tv_sec = 0; // timeout en segundos
		timeout.tv_usec = sleepEnemigos * 1000; //timeout en microsegundos

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
//			rnd(&x, MAXCOLS);
//			rnd(&y, MAXROWS);

//			rnd(&(enemy->enemigo.posicionActual.x), MAXCOLS);
//			rnd(&(enemy->enemigo.posicionActual.y), MAXROWS);

			moverEnemigo(enemy);

			verificarPosicionPersonajes(enemy);

			//gui_moverPersonaje(id, x, y );
			gui_moverPersonaje(id, enemy->enemigo.posicionActual.x, enemy->enemigo.posicionActual.y);
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

void verificarPosicionPersonajes(t_hiloEnemigo *enemy) {
	pthread_mutex_lock (&mutexListaPersonajesJugando);
	pthread_mutex_lock (&mutexListaPersonajesMuertosxEnemigo);

	t_personaje *personaje;
	int i;

	void _comparaCoordPJEnemigo(t_personaje *p) {
		log_info(LOGGER, " checarPosicionPersonajes: p(%d, %d) - e(%d, %d): dist=%d", p->posActual.x, p->posActual.y, enemy->enemigo.posicionActual.x, enemy->enemigo.posicionActual.y, calcularDistanciaCoord(p->posActual, enemy->enemigo.posicionActual) );
		if (calcularDistanciaCoord(p->posActual, enemy->enemigo.posicionActual) == 0)
		{
			enviarMsjPorPipe(enemy->fdPipeE2N[1], MUERTE_PERSONAJE_XENEMIGO);
			//agregarPersonajeMuertoxEnemigo(p);
			queue_push(listaPersonajesMuertosxEnemigo, p);
		}
	}

	list_iterate(listaPersonajesEnJuego, (void*)_comparaCoordPJEnemigo);

	bool _remove_x_id (t_personaje *p) {
		return (p->id == personaje->id);
	}

	for (i=0; i < queue_size(listaPersonajesMuertosxEnemigo); i++) {
		personaje = queue_pop(listaPersonajesMuertosxEnemigo);
		queue_push(listaPersonajesMuertosxEnemigo, personaje);

		list_remove_by_condition(listaPersonajesEnJuego, (void*)_remove_x_id);
	}

	pthread_mutex_unlock (&mutexListaPersonajesMuertosxEnemigo);
	pthread_mutex_unlock (&mutexListaPersonajesJugando);

}

//SECCION de FUNCIONES PARA EL MOVIMIENTO DE LOS ENEMIGOS
void moverEnemigo(t_hiloEnemigo* hiloEnemigo) {
	int32_t posX=0, posY=0;
	t_personaje PJ;
	t_posicion posicionPJ;
	int posValida=0;

	log_debug(LOGGER, "moverEnemigo: 1");

	if (list_size(listaPersonajesEnJuego))/* hay personajes en el nivel?*/
	{
		PJ = obternerPersonajeMasCercano(hiloEnemigo->enemigo.posicionActual);
		//pthread_mutex_unlock (&mutexListaPersonajesJugando);
		posicionPJ = PJ.posActual;
		moverEnemigoPorEje(hiloEnemigo, posicionPJ);

//		if ((posicionNueva.x == posicionPJ.x)&&(posicionNueva.y == posicionPJ.y)){
//			log_info(LOGGER, "El PJ '%c' ha sido alcanzado por un enemigo:",PJ.id);
//			//NOTIFICAR AL PLANIFICADOR que el personaje perdio una vida
//			// IMPRIMIR por pantalla GUI??
//		}
	}

	else{ //No hay personajes en el nivel
		log_debug(LOGGER, "moverEnemigo: 2 no hay personajes");
		if (
			((hiloEnemigo->enemigo.posicionActual.x) == (hiloEnemigo->enemigo.posicionEleSiguiente.x)) &&
			((hiloEnemigo->enemigo.posicionActual.y) == (hiloEnemigo->enemigo.posicionEleSiguiente.y))
			)
			{

			while (!posValida) {
				estimarMovimientoL(hiloEnemigo, &posX, &posY);
				posValida = validarPosicionEnemigo(hiloEnemigo, posX, posY);
				//posValida = 1;
			}
			
			hiloEnemigo->enemigo.posicionEleSiguiente.x = posX;
			hiloEnemigo->enemigo.posicionEleSiguiente.y = posY;
		}
		else{
				if(3>(calcularDistanciaCoord(hiloEnemigo->enemigo.posicionActual.y,hiloEnemigo->enemigo.posicionActual.x))){
					hiloEnemigo->enemigo.posicionActual.x = hiloEnemigo->enemigo.posicionEleSiguiente.x;
					hiloEnemigo->enemigo.posicionActual.y = hiloEnemigo->enemigo.posicionEleSiguiente.y;		
				}
			}
		moverDeUnoHacia(hiloEnemigo, &posX, &posY);
		//log_debug(LOGGER, "act:(%d, %d) - eleSig:(%d, %d)", hiloEnemigo->enemigo.posicionActual.x, hiloEnemigo->enemigo.posicionActual.y, hiloEnemigo->enemigo.posicionEleSiguiente.x, hiloEnemigo->enemigo.posicionEleSiguiente.y);
	}
}
//PARA QUE EL MOVIMIENTO SE REALICE DE A UNO POR VEZ
//SIEMPRE SE MUEVE PRIMERO EN X
void moverDeUnoHacia(t_hiloEnemigo* hiloEnemigo, int32_t* posX,int32_t* posY){
	if ((hiloEnemigo->enemigo.posicionActual.x) != hiloEnemigo->enemigo.posicionEleSiguiente.x) {
			if ((hiloEnemigo->enemigo.posicionActual.x) > hiloEnemigo->enemigo.posicionEleSiguiente.x){
						(hiloEnemigo->enemigo.posicionActual.x)--;
						}
			if ((hiloEnemigo->enemigo.posicionActual.x) < hiloEnemigo->enemigo.posicionEleSiguiente.x){
						(hiloEnemigo->enemigo.posicionActual.x)++;
						}
	}
	else {
		if ((hiloEnemigo->enemigo.posicionActual.y) > hiloEnemigo->enemigo.posicionEleSiguiente.y){
			(hiloEnemigo->enemigo.posicionActual.y)--;
			}
		if ((hiloEnemigo->enemigo.posicionActual.y) < hiloEnemigo->enemigo.posicionEleSiguiente.y){
			(hiloEnemigo->enemigo.posicionActual.y)++;
			}
	}
}

t_posicion moverEnemigoPorEje (t_hiloEnemigo* hiloEnemigo,t_posicion posicionHacia){
	t_posicion posicionNueva;
	if ((hiloEnemigo->enemigo.moverPorX) &&
			(hiloEnemigo->enemigo.posicionActual.x != posicionHacia.x)){
		posicionNueva = moverEnemigoEnX(hiloEnemigo, posicionHacia);
	}
	else{
		if(hiloEnemigo->enemigo.posicionActual.y != posicionHacia.y){
			posicionNueva = moverEnemigoEnY(hiloEnemigo, posicionHacia);
		}
	}
	return posicionNueva;
}

t_posicion moverEnemigoEnX(t_hiloEnemigo* hiloEnemigo, t_posicion posicionHacia){
	t_posicion posicionNueva;
	posicionNueva = (hiloEnemigo->enemigo.posicionActual);
	if ((posicionHacia.x) > (hiloEnemigo->enemigo.posicionActual.x)){
		(posicionNueva.x)++;
		if (validarPosicionEnemigo(hiloEnemigo, posicionNueva.x, posicionNueva.y)){
			(hiloEnemigo->enemigo.posicionActual.x)++;
		}
		else{//nuevo
		hiloEnemigo->enemigo.moverPorX = 1;
			(hiloEnemigo->enemigo.posicionActual.y)++;
		}

	}else{
		(posicionNueva.x)--;
		if (validarPosicionEnemigo(hiloEnemigo,posicionNueva.x,posicionNueva.y)){
			(hiloEnemigo->enemigo.posicionActual.x)--;
		}
	}
	hiloEnemigo->enemigo.moverPorX = 0;
	return posicionNueva;
}


t_posicion moverEnemigoEnY(t_hiloEnemigo* hiloEnemigo, t_posicion posicionHacia){
	t_posicion posicionNueva;
	posicionNueva = (hiloEnemigo->enemigo.posicionActual);
	if ((posicionHacia.y) > (hiloEnemigo->enemigo.posicionActual.y))
	{
		(posicionNueva.y)++;
		if(validarPosicionEnemigo(hiloEnemigo, posicionNueva.x, posicionNueva.y))
		{
			(hiloEnemigo->enemigo.posicionActual.y)++;
		}
	}
	else{
		(posicionNueva.y)--;
		if(validarPosicionEnemigo(hiloEnemigo,posicionNueva.x,posicionNueva.y))
		{
			(hiloEnemigo->enemigo.posicionActual.y)--;
		}
	}
	hiloEnemigo->enemigo.moverPorX = 1;
	return posicionNueva;
}

void estimarMovimientoL(t_hiloEnemigo* hiloEnemigo, int32_t* posX, int32_t* posY) {
	*posX = hiloEnemigo->enemigo.posicionActual.x;
	*posY = hiloEnemigo->enemigo.posicionActual.y;
	srand(time(NULL));
	int r = (rand()+hiloEnemigo->tid) % 8;

	switch(r) {
	case 1:
		(*posY)++; *posX = (*posX)+2;
		break;
	case 2:
		(*posY)++; *posX= (*posX)-2;
		break;
	case 3:
		(*posY)--; *posX= (*posX)+2;
		break;
	case 4:
		(*posY)--; *posX= (*posX)-2;
		break;
	case 5:
		*posY= (*posY)+2; (*posX)++;
		break;
	case 6:
		*posY= (*posY)-2; (*posX)++;
		break;
	case 7:
		*posY= (*posY)+2; (*posX)--;
		break;
	case 0:
		*posY= (*posY)-2; (*posX)--;
		break;
	}
}

t_personaje obternerPersonajeMasCercano(t_posicion miPosicion)
{
	pthread_mutex_lock (&mutexListaPersonajesJugando);
	t_personaje pjcercano;
	int32_t cant=0;
	t_posicion posMasCercana;
	int32_t distanciaMasCercana;

	posMasCercana.x = 1000;
	posMasCercana.y = 1000;
	cant = list_size(listaPersonajesEnJuego);
	distanciaMasCercana = calcularDistanciaCoord(miPosicion, posMasCercana);
	initPersonje(&pjcercano);

	void buscarPJcercano(t_personaje *p) {
		int32_t distancia = calcularDistanciaCoord(miPosicion, p->posActual);
		if (distanciaMasCercana > distancia)
		{
			posMasCercana = p->posActual;
			distanciaMasCercana = distancia;
			pjcercano.id = p->id;
			pjcercano.posActual = p->posActual;
		}
	}
	if(cant > 0){
		list_iterate(listaPersonajesEnJuego, (void*)buscarPJcercano);
	}
	pthread_mutex_unlock (&mutexListaPersonajesJugando);
	return pjcercano;
}

/*
int32_t posicionConItem(t_hiloEnemigo* hiloEnemigo, t_posicion posicion)
{
        int32_t hayCaja = 0;
        void hayItemEn(char *key, t_caja *caja)
        {
                if (posicion.x == caja->POSX && posicion.y == caja->POSY)
                {
                        hayCaja = 1;
                }
        }
        dictionary_iterator(listaPosicionesProhibidas, (void*)hayItemEn);

        return hayCaja;
}

int32_t validarPosicionEnemigo(t_hiloEnemigo* hiloEnemigo, int32_t X,int32_t Y) {
        if((X=0)&&(Y=0)){return 0;}
        t_posicion pos;
        pos.x=X;pos.y=Y;
        int32_t pAux=posicionConItem(hiloEnemigo,pos);
        if((Y < MAXROWS) && (X < MAXCOLS) && !pAux){
                return 1;//PosicionOK
        }
        return 0;// POSICION INVALIDA
}
*/
int32_t validarPosicionEnemigo(t_hiloEnemigo* hiloEnemigo, int32_t X,int32_t Y) {
        if((X<=0)||(Y<=0)){return 0;}// POSICION INVALIDA
        if((Y >= MAXROWS) || (X >= MAXCOLS)){return 0;}// POSICION INVALIDA
        t_posicion posicionConCaja[dictionary_size(listaPosicionesProhibidas)];
                int i;
                for (i=0; i<dictionary_size(listaPosicionesProhibidas);i++)
                        {
                        posicionConCaja[i].x = 100;
                        posicionConCaja[i].y = 100;
                        }
                i=0;

                void hayItemEn(char *key, t_caja *caja)
                        {
                        posicionConCaja[i].x = caja->POSX;
                        posicionConCaja[i].y = caja->POSY;
                        i++;
                        }
                dictionary_iterator(listaPosicionesProhibidas, (void*)hayItemEn);
        for (i=0; i<dictionary_size(listaPosicionesProhibidas);i++){
                log_debug(LOGGER, "                posicion de la cajita :p(%d,%d)",posicionConCaja[i].x ,posicionConCaja[i].y);
                        if((posicionConCaja[i].x == X )&& (posicionConCaja[i].y == Y)){

                                return 0;// POSICION INVALIDA
                        }

                }
        return 1;//PosicionOK
}
