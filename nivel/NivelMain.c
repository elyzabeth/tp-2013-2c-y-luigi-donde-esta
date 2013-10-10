/*
 * NivelMain.c
 *
 *  Created on: 22/09/2013
 *      Author: elyzabeth
 */

#include "NivelMain.h"


int main (int argc, char**argv) {

	// Correr tests
	if (argc > 1 && strcmp(argv[1], "-test")==0)
		return correrTest();

	// Registro signal and signal handler
	signal(SIGINT, signal_callback_handler);
	signal(SIGQUIT, signal_callback_handler);
	signal(SIGUSR1, signal_callback_handler);
	signal(SIGTERM, signal_callback_handler);

	inicializarNivel();
	principal ();
	//while(1);
	finalizarNivel();

	return EXIT_SUCCESS;
}



void principal () {
		int id_proceso = getpid();
		int sock = -1;

		int q, p;
		int x = 1;
		int y = 1;

		int ex1 = 10, ey1 = 14;
		int ex2 = 20, ey2 = 3;

		p = MAXCOLS;
		q = MAXROWS;

		log_info(LOGGER,"************** Iniciando Nivel '%s' (PID: %d) ***************\n", NOMBRENIVEL, id_proceso);
		// Conectar con proceso Plataforma
		conectar(configNivelPlataformaIp(), configNivelPlataformaPuerto(), &sock);
		if (enviarMSJNuevoNivel(sock) != EXITO){
			log_error(LOGGER, "ERROR en conexion con Plataforma");
			finalizarNivel();
			exit(EXIT_FAILURE);
		}



		CrearPersonaje(GUIITEMS, '@', p, q);
		CrearPersonaje(GUIITEMS, '#', x, y);

		nivel_gui_dibujar(GUIITEMS, NOMBRENIVEL);

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


			rnd(&ex1, MAXCOLS);
			rnd(&ey1, MAXROWS);
			rnd(&ex2, MAXCOLS);
			rnd(&ey2, MAXROWS);
			MoverPersonaje(GUIITEMS, '1', ex1, ey1 );
			MoverPersonaje(GUIITEMS, '2', ex2, ey2 );

			MoverPersonaje(GUIITEMS, '@', p, q);
			MoverPersonaje(GUIITEMS, '#', x, y);

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

			nivel_gui_dibujar(GUIITEMS, NOMBRENIVEL);

			if (key=='q' || key=='Q')
				break;
		}

		return;

}






