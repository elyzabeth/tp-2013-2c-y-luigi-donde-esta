/*
 * funcionesNivel.c
 *
 *  Created on: Oct 3, 2013
 *      Author: elyzabeth
 */

#include "funcionesNivel.h"


int correrTest() {
	// TODO llamar a funciones de cunit.
	return EXIT_SUCCESS;
}


void validarPosicionCaja(char s, int32_t x, int32_t y) {
	if(x < 0 || x > MAXCOLS || y < 0 || y > MAXROWS) {
		log_error(LOGGER, "ERROR AL CREAR CAJA '%c' POSICION (%d, %d) FUERA DE LOS LIMITES (%d, %d)", s, x, y, MAXCOLS, MAXROWS);
		perror("ERROR AL CREAR CAJA POSICION FUERA DE LOS LIMITES");
		finalizarNivel();
		exit(-1);
	}

}

void gui_crearPersonaje(char id, int x, int y) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	CrearPersonaje(GUIITEMS, id, x, y);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_crearCaja(char id, int x, int y, int instancias) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	CrearCaja(GUIITEMS, id, x, y, instancias);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_crearEnemigo(char id, int x, int y) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	CrearEnemigo(GUIITEMS, id, x, y);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_mover_personaje (char id, int x, int y) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	MoverPersonaje(GUIITEMS, id, x, y );
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}

void gui_dibujar() {
	pthread_mutex_lock (&mutexLockGlobalGUI);
	nivel_gui_dibujar(GUIITEMS, NOMBRENIVEL);
	pthread_mutex_unlock (&mutexLockGlobalGUI);
}


/**
 * @NAME: agregarCajasRecursos
 * @DESC: Agrega al listado de items las cajas
 * de recursos que figuran en el archivo de configuracion.
 */
void agregarCajasRecursos() {

	t_dictionary *recursos = configNivelRecursos();

	void _add_box(char *simbolo, t_caja *caja) {
		validarPosicionCaja(caja->SIMBOLO, caja->POSX, caja->POSY);
		gui_crearCaja(caja->SIMBOLO, caja->POSX, caja->POSY, caja->INSTANCIAS);
	}

	dictionary_iterator(recursos, (void*)_add_box);

}

/**
 * @NAME: agregarEnemigos
 * @DESC: Crea un hilo por cada enemigo.
 *  La cantidad de enemigos esta definida en el archivo de configuracion (Enemigos).
 */
void agregarEnemigos() {

	int i;
	int32_t cantEnemigos = configNivelEnemigos();
	int idEnemigo = '1';

	for(i=0; i < cantEnemigos; i++) {

		// Creo el hilo para el enemigo
		pthread_create (&idHiloEnemigo[i], NULL, (void*) enemigo, (int32_t*)idEnemigo);
		idEnemigo++;

	}

}


void inicializarNivelGui() {

	GUIITEMS = list_create();

	nivel_gui_inicializar();
    nivel_gui_get_area_nivel(&MAXROWS, &MAXCOLS);
    agregarEnemigos();
    agregarCajasRecursos();

    //nivel_gui_dibujar(GUIITEMS, NOMBRENIVEL);
    gui_dibujar();
}

void inicializarNivel () {
	// Levanto archivo de configuracion
	levantarArchivoConfiguracionNivel();
	strncpy(NOMBRENIVEL, configNivelNombre(), 20);

	// Creo LOGGER
	LOGGER = log_create(configNivelLogPath(), "NIVEL", configNivelLogConsola(), configNivelLogNivel() );
	log_info(LOGGER, "INFO: INICIALIZANDO NIVEL '%s'\n", NOMBRENIVEL);
	log_debug(LOGGER, "DEBUG: INICIALIZANDO NIVEL '%s' ", NOMBRENIVEL);

	pthread_mutex_init (&mutexLockGlobalGUI, NULL);
	//pthread_mutex_init (&mutexLockGlobalMoverPersonaje, NULL);

	//inicializar NIVEL-GUI
	inicializarNivelGui();

}

void finalizarHiloEnemigo() {
	int i ;

	log_info(LOGGER, "FINALIZANDO HILOS ENEMIGOS '%s'", NOMBRENIVEL);
	write (fdPipeMainToEnemy[1], "FIN", 4);

	for (i=0; i < configNivelEnemigos(); i++)
		pthread_join(idHiloEnemigo[i], NULL);
}

void finalizarNivel () {

	log_info(LOGGER, "FINALIZANDO NIVEL-GUI '%s'", NOMBRENIVEL);
	// Libero / finalizo NIVEL-GUI
	list_destroy_and_destroy_elements(GUIITEMS, (void*)free);
	nivel_gui_terminar();

	log_info(LOGGER, "FINALIZANDO NIVEL '%s'", NOMBRENIVEL);

	// libero Tuberia de comunicacion con hilos
	finalizarHiloEnemigo();
	close (fdPipeMainToEnemy[0]);
	close (fdPipeMainToEnemy[1]);

	pthread_mutex_destroy(&mutexLockGlobalGUI);
	//pthread_mutex_destroy(&mutexLockGlobalMoverPersonaje);

	// Libero estructuras de configuracion
	log_info(LOGGER, "LIBERANDO ESTRUCTURAS DE CONFIG-NIVEL '%s'", NOMBRENIVEL);
	destruirConfigNivel();

	// Libero logger
	log_info(LOGGER, "LIBERANDO ESTRUCTURAS DE LOGGER '%s' \n\n (Adios Mundo CRUEL!) piiiiiiiiiiiiiii.....\n\n", NOMBRENIVEL);
	log_destroy(LOGGER);

	// Libero a Willy!
	// free (Willy);
}


/*
 * @NAME: signal_callback_handler
 * @DESC: Define la funcion a llamar cuando una señal es enviada al proceso
 * ctrl-c (SIGINT)
 */
void signal_callback_handler(int signum)
{
	log_info(LOGGER, "INTERRUPCION POR SEÑAL: %d = %s \n", signum, strsignal(signum));

	switch(signum) {
		case SIGINT: // SIGINT=2 (ctrl-c)
			log_info(LOGGER, " - LLEGO SEÑAL SIGINT\n");
			finalizarNivel();
		break;
		case SIGUSR1: // SIGUSR1=10 ( kill -s USR1 <PID> )
			log_info(LOGGER, " - LLEGO SEÑAL SIGUSR1\n");
			finalizarNivel();
		break;
		case SIGTERM: // SIGTERM=15 ( kill <PID>)
			log_info(LOGGER, " - LLEGO SEÑAL SIGTERM\n");
			finalizarNivel();
		break;
		case SIGKILL: // SIGKILL=9 ( kill -9 <PID>)
			log_info(LOGGER, " - LLEGO SEÑAL SIGKILL\n");
			finalizarNivel();
		break;
		case SIGQUIT: // SIGQUIT=3 (ctrl-4 o kill -s QUIT <PID>)
			log_info(LOGGER, " - LLEGO SEÑAL SIGQUIT\n");
			finalizarNivel();
		break;
	}

	// Termino el programa
	exit(signum);
}


/*
 * @NAME: rnd
 * @DESC: Modifica el numero en +1,0,-1, sin pasarse del maximo dado
 */
void rnd(int *x, int max) {
	*x += (rand() % 3) - 1;
	*x = (*x<max) ? *x : max-1;
	*x = (*x>0) ? *x : 1;
}

int enviarMSJNuevoNivel(int sock) {
	header_t header;
	header.tipo = NUEVO_NIVEL;
	header.largo_mensaje = strlen(NOMBRENIVEL);
	char* bufferheader;
	char* bufferpayload = malloc(header.largo_mensaje+1);
	strcpy(bufferpayload, NOMBRENIVEL);

	bufferheader = calloc(1,sizeof(header_t)); /*primera y unica vez */
	memset(bufferheader, '\0', sizeof(header_t));
	memcpy(bufferheader, &header, sizeof(header_t));

	log_info(LOGGER, "sizeof(header): %d, largo mensaje: %d  bufferheader: %lu\n", sizeof(header), header.largo_mensaje, sizeof(&bufferheader));

	if (enviar(sock, bufferheader, sizeof(header_t)) != EXITO)
	{
		log_error(LOGGER,"Error al enviar header NUEVO_NIVEL\n\n");
		return WARNING;
	}

	if (enviar(sock, bufferpayload, header.largo_mensaje) != EXITO)
	{
		log_error(LOGGER,"Error al enviar NOMBRE NIVEL\n\n");
		return WARNING;
	}

	free(bufferpayload);
	free(bufferheader);
	return EXITO;
}


void simulacroJuego () {

	int q, p;
	int x = 1;
	int y = 1;
//	int ex1 = 10, ey1 = 14;
//	int ex2 = 20, ey2 = 3;

	p = MAXCOLS;
	q = MAXROWS;

	gui_crearPersonaje('@', p, q);
	gui_crearPersonaje('#', x, y);

	gui_dibujar();

	while ( 1 ) {
		int key = getch();

		switch( key ) {

			case KEY_UP:
				if (y > 1) {
					y--;
				}
			break;

			case KEY_DOWN:
				if (y < MAXROWS) {
					y++;
				}
			break;

			case KEY_LEFT:
				if (x > 1) {
					x--;
				}
			break;
			case KEY_RIGHT:
				if (x < MAXCOLS) {
					x++;
				}
			break;
			case 'w':
			case 'W':
				if (q > 1) {
					q--;
				}
			break;

			case 's':
			case 'S':
				if (q < MAXROWS) {
					q++;
				}
			break;

			case 'a':
			case 'A':
				if (p > 1) {
					p--;
				}
			break;
			case 'D':
			case 'd':
				if (p < MAXCOLS) {
					p++;
				}
			break;
			case 'Q':
			case 'q':
				//nivel_gui_terminar();
				//exit(0);
			break;
		}


//		rnd(&ex1, MAXCOLS);
//		rnd(&ey1, MAXROWS);
//		rnd(&ex2, MAXCOLS);
//		rnd(&ey2, MAXROWS);
//		MoverPersonaje(GUIITEMS, '1', ex1, ey1 );
//		MoverPersonaje(GUIITEMS, '2', ex2, ey2 );

		gui_mover_personaje('@', p, q);
		gui_mover_personaje('#', x, y);

		if (   ((p == 26) && (q == 10)) || ((x == 26) && (y == 10)) ) {
			restarRecurso(GUIITEMS, 'H');
		}

		if (   ((p == 19) && (q == 9)) || ((x == 19) && (y == 9)) ) {
			restarRecurso(GUIITEMS, 'F');
		}

		if (   ((p == 8) && (q == 15)) || ((x == 8) && (y == 15)) ) {
			restarRecurso(GUIITEMS, 'M');
		}

		if((p == x) && (q == y)) {
			BorrarItem(GUIITEMS, '#'); //si chocan, borramos uno (!)
		}

		gui_dibujar();

		if (key=='q' || key=='Q')
			break;
	}

	return;
}


void ejemploGui () {

	t_list* items = list_create();

		int rows, cols;
		int q, p;

		int x = 1;
		int y = 1;

		int ex1 = 10, ey1 = 14;
		int ex2 = 20, ey2 = 3;

		nivel_gui_inicializar();

	    nivel_gui_get_area_nivel(&rows, &cols);

		p = cols;
		q = rows;

		CrearPersonaje(items, '@', p, q);
		CrearPersonaje(items, '#', x, y);

		CrearEnemigo(items, '1', ex1, ey1);
		CrearEnemigo(items, '2', ex2, ey2);

		CrearCaja(items, 'H', 26, 10, 5);
		CrearCaja(items, 'M', 8, 15, 3);
		CrearCaja(items, 'F', 19, 9, 2);

		nivel_gui_dibujar(items, "Test Chamber 04");

		while ( 1 ) {
			int key = getch();

			switch( key ) {

				case KEY_UP:
					if (y > 1) {
						y--;
					}
				break;

				case KEY_DOWN:
					if (y < rows) {
						y++;
					}
				break;

				case KEY_LEFT:
					if (x > 1) {
						x--;
					}
				break;
				case KEY_RIGHT:
					if (x < cols) {
						x++;
					}
				break;
				case 'w':
				case 'W':
					if (q > 1) {
						q--;
					}
				break;

				case 's':
				case 'S':
					if (q < rows) {
						q++;
					}
				break;

				case 'a':
				case 'A':
					if (p > 1) {
						p--;
					}
				break;
				case 'D':
				case 'd':
					if (p < cols) {
						p++;
					}
				break;
				case 'Q':
				case 'q':
					//nivel_gui_terminar();
					//exit(0);
				break;
			}


			rnd(&ex1, cols);
			rnd(&ey1, rows);
			rnd(&ex2, cols);
			rnd(&ey2, rows);
			MoverPersonaje(items, '1', ex1, ey1 );
			MoverPersonaje(items, '2', ex2, ey2 );

			MoverPersonaje(items, '@', p, q);
			MoverPersonaje(items, '#', x, y);

			if (   ((p == 26) && (q == 10)) || ((x == 26) && (y == 10)) ) {
				restarRecurso(items, 'H');
			}

			if (   ((p == 19) && (q == 9)) || ((x == 19) && (y == 9)) ) {
				restarRecurso(items, 'F');
			}

			if (   ((p == 8) && (q == 15)) || ((x == 8) && (y == 15)) ) {
				restarRecurso(items, 'M');
			}

			if((p == x) && (q == y)) {
				BorrarItem(items, '#'); //si chocan, borramos uno (!)
			}

			nivel_gui_dibujar(items, "Test Chamber 04");

			if (key=='q' || key=='Q')
				break;
		}

		BorrarItem(items, '#');
		BorrarItem(items, '@');

		BorrarItem(items, '1');
		BorrarItem(items, '2');

		BorrarItem(items, 'H');
		BorrarItem(items, 'M');
		BorrarItem(items, 'F');

		//list_destroy_and_destroy_elements(items, (void*)free);

		nivel_gui_terminar();
}
