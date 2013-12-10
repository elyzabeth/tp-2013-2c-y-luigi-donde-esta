/*
 * enemigo.c
 *
 * Created on: Oct 11, 2013
 * Author: elizabeth
 */


#include "funcionesNivel.h"


t_dictionary *listaPosicionesProhibidas; // = configNivelRecursos();


void moverEnemigo(t_hiloEnemigo* hiloEnemigo);
//void moverDeUnoHacia(t_hiloEnemigo* hiloEnemigo,int32_t* posX,int32_t* posY);
void estimarMovimientoL(t_hiloEnemigo* hiloEnemigo, int32_t* x,int32_t* y);
t_personaje obternerPersonajeMasCercano(t_posicion miPosicion);
int32_t validarPosicionEnemigo(t_hiloEnemigo* hiloEnemigo, int32_t X,int32_t Y);
t_posicion moverEnemigoHaciaPJ (t_hiloEnemigo* hiloEnemigo,t_posicion posicionHacia);
t_posicion moverEnemigoEnL (t_hiloEnemigo* hiloEnemigo,t_posicion posicionHacia);
void verificarPosicionPersonajes(t_hiloEnemigo *enemy);



void* enemigo (t_hiloEnemigo *enemy) {
	listaPosicionesProhibidas = configNivelRecursos();
	int32_t id = (int32_t) enemy->id;
	int32_t sleepEnemigos;
	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;
	int fin = false;
	struct timeval timeout;

	log_info(LOGGER, "Enemigo '%c' Iniciado.", id);

	// Obtengo parametro del archivo de configuracion
	sleepEnemigos = configNivelSleepEnemigos();

	FD_ZERO(&master);

	// Agrego descriptor del Pipe con Nivel.
	agregar_descriptor(enemy->fdPipe[0], &master, &max_desc);


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

		timeout.tv_sec = 0; // sleepEnemigos * 0.001; timeout en segundos
		timeout.tv_usec = sleepEnemigos * 1000; //timeout en microsegundos

		ret = select(max_desc+1, &read_fds, NULL, NULL, &timeout);
		if(ret == -1) {
			printf("Enemigo '%c': ERROR en select.", id);
			sleep(1);
			continue;
		}
		if (ret == 0) {

			moverEnemigo(enemy);

			verificarPosicionPersonajes(enemy);

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



// SECCION de FUNCIONES PARA EL MOVIMIENTO DE LOS ENEMIGOS
// ******************************************************

void verificarPosicionPersonajes(t_hiloEnemigo *enemy) {
	pthread_mutex_lock (&mutexListaPersonajesJugando);
	pthread_mutex_lock (&mutexListaPersonajesMuertosxEnemigo);

	t_personaje *personaje;
	int i;
	char msj[100]={0};

	void _comparaCoordPJEnemigo(t_personaje *p) {
		//log_debug(LOGGER, " verificarPosicionPersonajes: p(%d, %d) - e(%d, %d): dist=%d", p->posActual.x, p->posActual.y, enemy->enemigo.posicionActual.x, enemy->enemigo.posicionActual.y, calcularDistanciaCoord(p->posActual, enemy->enemigo.posicionActual) );
		if (calcularDistanciaCoord(p->posActual, enemy->enemigo.posicionActual) == 0)
		{
			//agregarPersonajeMuertoxEnemigo(p);
			queue_push(listaPersonajesMuertosxEnemigo, p);
			enviarMsjPorPipe(enemy->fdPipeE2N[1], MUERTE_PERSONAJE_XENEMIGO);
			sprintf(msj, "%s - Muerte Personaje: %s", NOMBRENIVEL, p->nombre);
			gui_dibujarEnemigo(msj);
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



void moverEnemigo(t_hiloEnemigo* hiloEnemigo) {
	int32_t posX=0, posY=0;
	t_personaje PJ;
	t_posicion posicionPJ;
	int posValida=0;
	int hayPersonajes = obtenerCantPersonajesEnJuego();

	// Si hay personajes en el nivel el movimiento es hacia el personaje mas cercano.
	//if (list_size(listaPersonajesEnJuego))
	if(hayPersonajes)
	{
		PJ = obternerPersonajeMasCercano(hiloEnemigo->enemigo.posicionActual);
		posicionPJ = PJ.posActual;

		// Si el personaje mas cercano No esta en una posicion prohibida
		if (validarPosicionEnemigo(hiloEnemigo, posicionPJ.x, posicionPJ.y))
		{
			//moverEnemigoPorEje(hiloEnemigo, posicionPJ);
			moverEnemigoHaciaPJ(hiloEnemigo, posicionPJ);
		} else {

			// Opcion 1: Si el personaje esta en posicion prohibida cambio el eje por el que se debe mover
			//hiloEnemigo->enemigo.moverPorX = !hiloEnemigo->enemigo.moverPorX;

			// Opcion 2: Si el personaje esta en posicion prohibida fuerzo movimiento en L
			hayPersonajes = 0;
		}
	}

	// Si No hay personajes en el nivel el movimiento es en L
	if(!hayPersonajes) {

		// Si ya llego a la ultima posicion del movimiento en L
		// Debo calcular el proximo movimento en L
		if ((hiloEnemigo->enemigo.posicionActual.x == hiloEnemigo->enemigo.posicionEleSiguiente.x) &&
			(hiloEnemigo->enemigo.posicionActual.y == hiloEnemigo->enemigo.posicionEleSiguiente.y)
			)
		{

			while (!posValida) {
				// Estimo proxima posicion en L y valido que no este en ninguna coordenada prohibida.
				estimarMovimientoL(hiloEnemigo, &posX, &posY);
				posValida = validarPosicionEnemigo(hiloEnemigo, posX, posY);
			}
			
			hiloEnemigo->enemigo.posicionEleSiguiente.x = posX;
			hiloEnemigo->enemigo.posicionEleSiguiente.y = posY;

		} else {
			if( 3 < (calcularDistanciaCoord(hiloEnemigo->enemigo.posicionActual,hiloEnemigo->enemigo.posicionEleSiguiente))){
				hiloEnemigo->enemigo.posicionEleSiguiente.x = hiloEnemigo->enemigo.posicionActual.x;
				hiloEnemigo->enemigo.posicionEleSiguiente.y = hiloEnemigo->enemigo.posicionActual.y;
			}
		}

		moverEnemigoEnL(hiloEnemigo, hiloEnemigo->enemigo.posicionEleSiguiente);
		// TODO ver porque en el movimiento de a uno pasa por coordenadas prohibidas
		//moverDeUnoHacia(hiloEnemigo, &posX, &posY);
		//log_debug(LOGGER, "act:(%d, %d) - eleSig:(%d, %d)", hiloEnemigo->enemigo.posicionActual.x, hiloEnemigo->enemigo.posicionActual.y, hiloEnemigo->enemigo.posicionEleSiguiente.x, hiloEnemigo->enemigo.posicionEleSiguiente.y);

	}

}


/*
 * moverEnemigoHaciaPJ
 * Mueve al enemigo hacia el personaje alternando ejes x/y
 * y valida que la nueva posicion no sea una coordenada prohibida.
 */
t_posicion moverEnemigoHaciaPJ (t_hiloEnemigo* hiloEnemigo,t_posicion posicionHacia) {
	t_posicion posicionNueva;
	int flag = false;
	int posValida = 0;
	posicionNueva = hiloEnemigo->enemigo.posicionActual;

	log_debug(LOGGER, "e%c: (%d,%d) - pj: (%d,%d) - moverPorX: %d", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y, hiloEnemigo->enemigo.moverPorX );
	while(!posValida) {
		//flag = false;
		posicionNueva = hiloEnemigo->enemigo.posicionActual;

		if (hiloEnemigo->enemigo.moverPorX || hiloEnemigo->enemigo.posicionActual.y == posicionHacia.y) {
			if(hiloEnemigo->enemigo.posicionActual.x < posicionHacia.x ) {
				//posicionNueva.x = hiloEnemigo->enemigo.posicionActual.x+1;
				posicionNueva.x++;
				log_debug(LOGGER, "posNueva1: e%c: (%d,%d) - pj: (%d,%d)", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y );
				flag = true;
			} else if(hiloEnemigo->enemigo.posicionActual.x > posicionHacia.x ) {
				//posicionNueva.x = hiloEnemigo->enemigo.posicionActual.x-1;
				posicionNueva.x--;
				log_debug(LOGGER, "posNueva2: e%c: (%d,%d) - pj: (%d,%d)", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y );
				flag = true;
			}

		}
		if (!flag) {
			if ((!hiloEnemigo->enemigo.moverPorX || hiloEnemigo->enemigo.posicionActual.x == posicionHacia.x)) {
				if(hiloEnemigo->enemigo.posicionActual.y < posicionHacia.y ){
					//hiloEnemigo->enemigo.posicionActual.y++;
					//posicionNueva.y = hiloEnemigo->enemigo.posicionActual.y+1;
					posicionNueva.y++;
					log_debug(LOGGER, "posNueva3: e%c: (%d,%d) - pj: (%d,%d)", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y );
				} else if(hiloEnemigo->enemigo.posicionActual.y > posicionHacia.y ){
					//hiloEnemigo->enemigo.posicionActual.y--;
					//posicionNueva.y = hiloEnemigo->enemigo.posicionActual.y-1;
					posicionNueva.y--;
					log_debug(LOGGER, "posNueva4: e%c: (%d,%d) - pj: (%d,%d)", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y );
				}
			}
		}
		hiloEnemigo->enemigo.moverPorX = !hiloEnemigo->enemigo.moverPorX;
		if (!(posValida = validarPosicionEnemigo(hiloEnemigo, posicionNueva.x, posicionNueva.y)))
			flag = !flag;
	}

	hiloEnemigo->enemigo.posicionActual = posicionNueva;

	return posicionNueva;
}


t_posicion moverEnemigoEnL (t_hiloEnemigo* hiloEnemigo,t_posicion posicionHacia) {
	t_posicion posicionNueva;
	int flag = true;
	int posValida = 0;
	posicionNueva = hiloEnemigo->enemigo.posicionActual;
	int i;
	char eje;
	hiloEnemigo->enemigo.imovL = hiloEnemigo->enemigo.imovL > 2 ? 0 : hiloEnemigo->enemigo.imovL;
	i = hiloEnemigo->enemigo.imovL;
	eje = hiloEnemigo->enemigo.movL[i];

	while(!posValida) {

		posicionNueva = hiloEnemigo->enemigo.posicionActual;
		log_debug(LOGGER, "L - e%c: (%d,%d) - dest: (%d,%d) - movL: %s - movL[%d]: %c", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y, hiloEnemigo->enemigo.movL, i, eje);

		if (flag) {
			if (eje == 'x')
			{
				if(hiloEnemigo->enemigo.posicionActual.x < posicionHacia.x ) {
					posicionNueva.x++;
					log_debug(LOGGER, "L - posNueva1: e%c: (%d,%d) - dest: (%d,%d) - moverPorX: %d", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y, hiloEnemigo->enemigo.moverPorX );
					hiloEnemigo->enemigo.moverPorX = true;
				} else if(hiloEnemigo->enemigo.posicionActual.x > posicionHacia.x ) {
					posicionNueva.x--;
					log_debug(LOGGER, "L - posNueva2: e%c: (%d,%d) - dest: (%d,%d) - moverPorX: %d", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y, hiloEnemigo->enemigo.moverPorX );
					hiloEnemigo->enemigo.moverPorX = true;
				} else {
					flag = false;
				}
			}

			if (eje == 'y') {
				if(hiloEnemigo->enemigo.posicionActual.y < posicionHacia.y ){
					posicionNueva.y++;
					log_debug(LOGGER, "L - posNueva3: e%c: (%d,%d) - pj: (%d,%d) - moverPorX: %d", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y, hiloEnemigo->enemigo.moverPorX );
					hiloEnemigo->enemigo.moverPorX = false;
				} else if(hiloEnemigo->enemigo.posicionActual.y > posicionHacia.y ){
					posicionNueva.y--;
					log_debug(LOGGER, "L - posNueva4: e%c: (%d,%d) - pj: (%d,%d) - moverPorX: %d", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y, hiloEnemigo->enemigo.moverPorX );
					hiloEnemigo->enemigo.moverPorX = false;
				}
			}
		} else {

			if (eje == 'y')
			{
				if(hiloEnemigo->enemigo.posicionActual.x < posicionHacia.x ) {
					posicionNueva.x++;
					log_debug(LOGGER, "L - posNueva1: e%c: (%d,%d) - pj: (%d,%d) - moverPorX: %d", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y, hiloEnemigo->enemigo.moverPorX );
					hiloEnemigo->enemigo.moverPorX = true;
				} else if(hiloEnemigo->enemigo.posicionActual.x > posicionHacia.x ) {
					posicionNueva.x--;
					log_debug(LOGGER, "L - posNueva2: e%c: (%d,%d) - pj: (%d,%d) - moverPorX: %d", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y, hiloEnemigo->enemigo.moverPorX );
					hiloEnemigo->enemigo.moverPorX = true;
				} else {
					break;
				}
			}

			if (eje == 'x') {
				if(hiloEnemigo->enemigo.posicionActual.y < posicionHacia.y ){
					posicionNueva.y++;
					log_debug(LOGGER, "L - posNueva3: e%c: (%d,%d) - pj: (%d,%d) - moverPorX: %d", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y, hiloEnemigo->enemigo.moverPorX );
					hiloEnemigo->enemigo.moverPorX = false;
				} else if(hiloEnemigo->enemigo.posicionActual.y > posicionHacia.y ){
					posicionNueva.y--;
					log_debug(LOGGER, "L - posNueva4: e%c: (%d,%d) - pj: (%d,%d) - moverPorX: %d", hiloEnemigo->id, posicionNueva.x, posicionNueva.y, posicionHacia.x, posicionHacia.y, hiloEnemigo->enemigo.moverPorX );
					hiloEnemigo->enemigo.moverPorX = false;
				} else {
					break;
				}
			}
		}

		if (!(posValida = validarPosicionEnemigo(hiloEnemigo, posicionNueva.x, posicionNueva.y))){
			flag = !flag;
			//hiloEnemigo->enemigo.imovL = 0;
		}

	}

	if (posValida) {
		hiloEnemigo->enemigo.posicionActual = posicionNueva;
		hiloEnemigo->enemigo.imovL++;
	} else {
		posicionNueva = hiloEnemigo->enemigo.posicionActual;
		hiloEnemigo->enemigo.posicionEleSiguiente = hiloEnemigo->enemigo.posicionActual;
	}

	return posicionNueva;
}


////PARA QUE EL MOVIMIENTO SE REALICE DE A UNO POR VEZ
////SIEMPRE SE MUEVE PRIMERO EN X
//void moverDeUnoHacia(t_hiloEnemigo* hiloEnemigo, int32_t* posX,int32_t* posY){
//	if ((hiloEnemigo->enemigo.posicionActual.x) != hiloEnemigo->enemigo.posicionEleSiguiente.x) {
//			if ((hiloEnemigo->enemigo.posicionActual.x) > hiloEnemigo->enemigo.posicionEleSiguiente.x){
//						(hiloEnemigo->enemigo.posicionActual.x)--;
//						}
//			if ((hiloEnemigo->enemigo.posicionActual.x) < hiloEnemigo->enemigo.posicionEleSiguiente.x){
//						(hiloEnemigo->enemigo.posicionActual.x)++;
//						}
//	}
//	else {
//		if ((hiloEnemigo->enemigo.posicionActual.y) > hiloEnemigo->enemigo.posicionEleSiguiente.y){
//			(hiloEnemigo->enemigo.posicionActual.y)--;
//			}
//		if ((hiloEnemigo->enemigo.posicionActual.y) < hiloEnemigo->enemigo.posicionEleSiguiente.y){
//			(hiloEnemigo->enemigo.posicionActual.y)++;
//			}
//	}
//}


void estimarMovimientoL(t_hiloEnemigo* hiloEnemigo, int32_t* posX, int32_t* posY) {
	*posX = hiloEnemigo->enemigo.posicionActual.x;
	*posY = hiloEnemigo->enemigo.posicionActual.y;
	srand(time(NULL));
	int r = (rand()+hiloEnemigo->tid+hiloEnemigo->enemigo.id) % 16;
	hiloEnemigo->enemigo.imovL = 0;

	switch(r) {
		case 0:
			(*posY)++; *posX = (*posX)+2;
			strcpy(hiloEnemigo->enemigo.movL, "xxy");
			break;
		case 1:
			(*posY)++; *posX= (*posX)-2;
			strcpy(hiloEnemigo->enemigo.movL, "xxy");
			break;
		case 2:
			(*posY)--; *posX= (*posX)+2;
			strcpy(hiloEnemigo->enemigo.movL, "xxy");
			break;
		case 3:
			(*posY)--; *posX= (*posX)-2;
			strcpy(hiloEnemigo->enemigo.movL, "xxy");
			break;
		case 4:
			*posY= (*posY)+2; (*posX)++;
			strcpy(hiloEnemigo->enemigo.movL, "yyx");
			break;
		case 5:
			*posY= (*posY)-2; (*posX)++;
			strcpy(hiloEnemigo->enemigo.movL, "yyx");
			break;
		case 6:
			*posY= (*posY)+2; (*posX)--;
			strcpy(hiloEnemigo->enemigo.movL, "yyx");
			break;
		case 7:
			*posY= (*posY)-2; (*posX)--;
			strcpy(hiloEnemigo->enemigo.movL, "yyx");
			break;


		case 8:
			(*posY)++; *posX = (*posX)+2;
			strcpy(hiloEnemigo->enemigo.movL, "yxx");
			break;
		case 9:
			(*posY)++; *posX= (*posX)-2;
			strcpy(hiloEnemigo->enemigo.movL, "yxx");
			break;
		case 10:
			(*posY)--; *posX= (*posX)+2;
			strcpy(hiloEnemigo->enemigo.movL, "yxx");
			break;
		case 11:
			(*posY)--; *posX= (*posX)-2;
			strcpy(hiloEnemigo->enemigo.movL, "yxx");
			break;
		case 12:
			*posY= (*posY)+2; (*posX)++;
			strcpy(hiloEnemigo->enemigo.movL, "xyy");
			break;
		case 13:
			*posY= (*posY)-2; (*posX)++;
			strcpy(hiloEnemigo->enemigo.movL, "xyy");
			break;
		case 14:
			*posY= (*posY)+2; (*posX)--;
			strcpy(hiloEnemigo->enemigo.movL, "xyy");
			break;
		case 15:
			*posY= (*posY)-2; (*posX)--;
			strcpy(hiloEnemigo->enemigo.movL, "xyy");
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


int32_t validarPosicionEnemigo(t_hiloEnemigo* hiloEnemigo, int32_t X,int32_t Y) {
//        if((X<=0)||(Y<=0)){return 0;}// POSICION INVALIDA
//        if((Y >= MAXROWS) || (X >= MAXCOLS)){return 0;}// POSICION INVALIDA
	if( X<=0 || Y<=0 || Y >= MAXROWS || X >= MAXCOLS ){return 0;}// POSICION INVALIDA
	int i;

	for (i=0; i<totalCoordProhibidas; i++) {
       	//log_debug(LOGGER, "    posicion de la cajita: (%d,%d) - enemigo: (%d,%d)",coordProhibidas[i].x ,coordProhibidas[i].y, X, Y);

       	if((coordProhibidas[i].x == X ) && (coordProhibidas[i].y == Y)) {
			return 0;// POSICION INVALIDA
		}
	}

	return 1;//PosicionOK
}
