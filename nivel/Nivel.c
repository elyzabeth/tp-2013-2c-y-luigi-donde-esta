/*
 * nivel.c
 *
 *  Created on: 22/09/2013
 *      Author: elyzabeth
 */

#include "Nivel.h"


int main (int argc, char**argv) {

	// Correr tests
	if (argc > 1 && strcmp(argv[1], "-test")==0)
		return correrTest();

	inicializar ();
	principal ();
	finalizar ();

	return EXIT_SUCCESS;
}

int correrTest() {
	// TODO llamar a funciones de cunit.
	return EXIT_SUCCESS;
}

void principal () {

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
					nivel_gui_terminar();
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

		list_destroy_and_destroy_elements(items, (void*)free);

		nivel_gui_terminar();

}


void inicializar () {
	levantarArchivoConfiguracionNivel();
	LOGGER = log_create(configNivelLogPath(), "NIVEL", configNivelLogConsola(), configNivelLogNivel() );
	strncpy(NOMBRENIVEL, configNivelNombre(), 20);
	log_info(LOGGER, "INFO: INICIALIZANDO NIVEL '%s' ", NOMBRENIVEL);

	log_debug(LOGGER, "DEBUG: INICIALIZANDO NIVEL '%s' ", NOMBRENIVEL);
}

void finalizar () {
	log_info(LOGGER, "FINALIZANDO NIVEL '%s'", NOMBRENIVEL);
	destruirConfigNivel();
	log_destroy(LOGGER);
}

/*
 * @NAME: rnd
 * @DESC: Modifica el numero en +1,0,-1, sin pasarse del maximo dado
 */
void rnd(int *x, int max){
	*x += (rand() % 3) - 1;
	*x = (*x<max) ? *x : max-1;
	*x = (*x>0) ? *x : 1;
}


