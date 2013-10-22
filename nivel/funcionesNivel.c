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

// FUNCIONES de la interfaz grafica sincronizadas con semaforo mutex.
// Porque se acceden consurrentemente desde varios hilos.
void gui_borrarItem(char id) {
	pthread_mutex_lock (&mutexLockGlobalGUI);
    bool _search_by_id(ITEM_NIVEL* item) {
        return item->id == id;
    }
    free(list_remove_by_condition(GUIITEMS, (void*) _search_by_id));
	pthread_mutex_unlock (&mutexLockGlobalGUI);
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

void gui_moverPersonaje (char id, int x, int y) {
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
 * @NAME: validarPosicionCaja
 * @DESC: Valida que la caja de recursos que se va a agregar este dentro de los limites.
 */
void validarPosicionCaja(char s, int32_t x, int32_t y) {
	if(x < 0 || x > MAXCOLS || y < 0 || y > MAXROWS) {
		log_error(LOGGER, "ERROR AL CREAR CAJA '%c' POSICION (%d, %d) FUERA DE LOS LIMITES (%d, %d)", s, x, y, MAXCOLS, MAXROWS);
		perror("ERROR AL CREAR CAJA POSICION FUERA DE LOS LIMITES");
		finalizarNivel();
		exit(-1);
	}

}


/**
 * @NAME: agregarCajasRecursos
 * @DESC: Agrega al listado de items las cajas
 * de recursos que figuran en el archivo de configuracion.
 */
void agregarCajasRecursos() {
	// Diccionario de recursos con clave=simbolo data=t_caja
	//t_dictionary *recursos = configNivelRecursos();

	void _add_box(char *simbolo, t_caja *caja) {
		validarPosicionCaja(caja->SIMBOLO, caja->POSX, caja->POSY);
		gui_crearCaja(caja->SIMBOLO, caja->POSX, caja->POSY, caja->INSTANCIAS);
	}

	dictionary_iterator(listaRecursos, (void*)_add_box);

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
	t_hiloEnemigo *enemy;

	for(i=0; i < cantEnemigos; i++) {
		enemy = crearEnemigo(idEnemigo);

		// Creo el hilo para el enemigo
		pthread_create (&enemy->tid, NULL, (void*) enemigo, (t_hiloEnemigo*)enemy);
		log_info(LOGGER, "agregarEnemigos: idHiloEnemigo: %u", enemy->tid);
		list_add(listaEnemigos, enemy);
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
	log_info(LOGGER, " ********************* INICIALIZANDO NIVEL '%s' ***************************\n", NOMBRENIVEL);

	// Inicializo estructura de hilo de deteccion de interbloqueo
	memset(&hiloInterbloqueo,'\0',sizeof(t_hiloInterbloqueo));
	pipe(hiloInterbloqueo.fdPipe);

	pthread_mutex_init (&mutexLockGlobalGUI, NULL);

	// inicializo listas
	listaEnemigos = list_create();
	listaRecursos = configNivelRecursos();

	// inicializo inotify
	notifyFD = crearNotifyFD();
	watchDescriptor = inotify_add_watch(notifyFD, PATH_CONFIG_NIVEL, IN_MODIFY);

	//inicializar NIVEL-GUI
	inicializarNivelGui();

}

/**
 * @NAME: finalizarHilosEnemigos
 * @DESC: Finaliza los hilos creados para cada enemigo enviandoles por pipe el mensaje FINALIZAR
 */
void finalizarHilosEnemigos() {
	int i = 0;
	int32_t cantEnemigos = configNivelEnemigos();
	header_t header;
	char* buffer_header = calloc(1,sizeof(header_t));

	log_info(LOGGER, "FINALIZANDO HILOS ENEMIGOS '%s'", NOMBRENIVEL);

	initHeader(&header);
	header.tipo = FINALIZAR;
	header.largo_mensaje=0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	void _finalizar_hilo(t_hiloEnemigo *enemy) {
		log_debug(LOGGER, "%d/%d) Envio mensaje de FINALIZAR a Enemigo '%c' (%u)", ++i, cantEnemigos, enemy->id, enemy->tid);
		write(enemy->fdPipe[1], buffer_header, sizeof(header_t));
		pthread_join(enemy->tid, NULL);
		sleep(1);
	}

	list_iterate(listaEnemigos, (void*)_finalizar_hilo);

	free(buffer_header);
}

void finalizarNivel () {

	log_info(LOGGER, "FINALIZANDO NIVEL-GUI '%s'", NOMBRENIVEL);

	// finalizo hilos enemigos
	finalizarHilosEnemigos();

	// finalizo hilo Interbloqueo
	enviarMsjAInterbloqueo(FINALIZAR);

	// Libero / finalizo NIVEL-GUI
	nivel_gui_terminar();

	log_info(LOGGER, "FINALIZANDO NIVEL '%s'", NOMBRENIVEL);

	// Libero listas dinamicas
	list_destroy_and_destroy_elements(GUIITEMS, (void*)free);
	list_destroy_and_destroy_elements(listaEnemigos, (void*)destruirEnemigo);
	dictionary_destroy_and_destroy_elements(listaRecursos, (void*)destruirCaja);

	// Finalizo inotify
	inotify_rm_watch(notifyFD, watchDescriptor);
	close(notifyFD);

	// libero semaforos
	pthread_mutex_destroy(&mutexLockGlobalGUI);

	// Libero estructuras de configuracion
	log_info(LOGGER, "LIBERANDO ESTRUCTURAS DE CONFIG-NIVEL '%s'", NOMBRENIVEL);
	destruirConfigNivel();

	// Libero logger
	log_info(LOGGER, "LIBERANDO ESTRUCTURAS DE LOGGER '%s' \n\n (Adios Mundo CRUEL!) piiiiiiiiiiiiiii.....\n--------------------------------------\n", NOMBRENIVEL);
	log_destroy(LOGGER);

	// Libero a Willy!
	// free (Willy);
}

int crearNotifyFD() {

	int fd;

	/*creating the INOTIFY instance*/
	fd = inotify_init();

	/*checking for error*/
	if (fd < 0) {
		perror("inotify_init");
	}

	return fd;
}


/*
 * @NAME: signal_callback_handler
 * @DESC: Define la funcion a llamar cuando una señal es enviada al proceso
 * ej: ctrl-c (SIGINT)
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

int enviarMsjAInterbloqueo (char msj) {
	int ret;
	header_t header;
	char* buffer_header = malloc(sizeof(header_t));

	memset(&header, '\0', sizeof(header_t));
	header.tipo = msj;
	header.largo_mensaje=0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	log_info(LOGGER, "Enviando mensaje al orquestador.");

	ret =  write(hiloInterbloqueo.fdPipe[1], buffer_header, sizeof(header_t));
	pthread_join(hiloInterbloqueo.tid, NULL);

	free(buffer_header);

	return ret;
}

int enviarMSJNuevoNivel(int sock) {
	header_t header;
	t_nivel nivel;
	char* buffer_header;
	char* buffer_payload;

	initHeader(&header);
	header.tipo = NUEVO_NIVEL;
	header.largo_mensaje = sizeof(t_nivel);

	buffer_header = calloc(1, sizeof(header_t));
	//memset(bufferheader, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	log_info(LOGGER, "enviarMSJNuevoNivel: sizeof(header): %d, largo mensaje: %d  bufferheader: %lu\n", sizeof(header), header.largo_mensaje, sizeof(&buffer_header));

	if (enviar(sock, buffer_header, sizeof(header_t)) != EXITO)
	{
		log_error(LOGGER,"enviarMSJNuevoNivel: Error al enviar header NUEVO_NIVEL\n\n");
		return WARNING;
	}

	//memset(&nivel, '\0', sizeof(t_nivel));
	initNivel(&nivel);
	strcpy(nivel.nombre, configNivelNombre());
	strcpy(nivel.algoritmo, configNivelAlgoritmo());
	nivel.quantum = configNivelQuantum();
	nivel.retardo = configNivelRetardo();

	buffer_payload = calloc(1,sizeof(t_nivel));
	memcpy(buffer_payload, &nivel, sizeof(t_nivel));

	if (enviar(sock, buffer_payload, header.largo_mensaje) != EXITO)
	{
		log_error(LOGGER,"enviarMSJNuevoNivel: Error al enviar datos del nivel\n\n");
		return WARNING;
	}

	free(buffer_payload);
	free(buffer_header);

	return EXITO;
}


void tratarSolicitudUbicacion(int sock, header_t header) {
	t_personaje personaje;
	char* buffer_payload;
	buffer_payload = calloc(1, header.largo_mensaje);

	// Si llega un mensaje de SOLICITUD_UBICACION luego espero recibir un t_personaje
	if (recibir(sock, buffer_payload, header.largo_mensaje) != EXITO)
	{
		log_error(LOGGER,"tratarSolicitudUbicacion: ERROR al recibir payload SOLICITUD_UBICACION\n\n");
		free(buffer_payload);
		// TODO cancelo o solo retorno??
		return;
	}

	initPersonje(&personaje);
	memcpy(&personaje, buffer_payload, sizeof(t_personaje));

	// TODO agegar personaje a lista de personajes en juego
	// y a la lista GUIITEMS para graficarlo.
	gui_crearPersonaje(personaje.id, personaje.posActual.x, personaje.posActual.y);

	free(buffer_payload);

}


// FUNCIONES DE PRUEBA - BORRAR CUANDO YA NO SE USEN (simulacroJuego y ejemploGui)
// -------------------------------------------------------------------------------
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

		gui_moverPersonaje('@', p, q);
		gui_moverPersonaje('#', x, y);

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
