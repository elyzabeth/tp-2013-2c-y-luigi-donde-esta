/*
 * interbloqueo.c
 *
 *  Created on: Oct 11, 2013
 *      Author: elizabeth
 */

#include "funcionesNivel.h"

#define MAXPER 20
#define MAXREC 20

int32_t T[MAXREC]; // vector temporal de disponibles
int32_t matAsignacion[MAXPER][MAXREC]; // Asignacion [personaje][recurso]
int32_t matSolicitud[MAXPER][MAXREC]; // Solicitud [personaje][recurso]
int32_t vecDisponibles[MAXREC];
int32_t vecPersonajesEnNivel[MAXPER][2];
int32_t vecRecursos[MAXREC];
int32_t vecRecursosCantTotal[MAXREC];
int32_t totalRecursos;
int32_t totalPersonajes;
char interbloqueados[MAXPER];



// Prototipos de funciones del hilo
int32_t detectarDeadlock();
t_personaje* recovery(int32_t cantInterbloqueados);

int initMatrices();
int imprimirMatrices();
int obtenerPosPersonaje(char p);
int obtenerPosRecurso(char r);
int llenarVecPJEnNivel(t_queue *listaPJEnNivel);
int llenarRecursos(t_dictionary *dicRecursos);
int llenarMatAsignacion(t_dictionary *recursosxPJ);
int llenarMatSolicitud(t_list *listaPJBloqueados);

int marcarPersonajesSinRecursosAsig();
int copiarDisponiblesAT();
int marcarNoBloqueados();
int contarPersonajesSinMarcar();

void* interbloqueo(t_hiloInterbloqueo *hiloInterbloqueoo) {
	header_t header;
	fd_set master;
	fd_set read_fds;

	int max_desc = 0;
	int i, ret;
	struct timeval timeout;
	int fin = false;

	int32_t TiempoChequeoDeadlock;
	int32_t RecoveryOn;
	int32_t hayDeadLock;

	log_info(LOGGER, "HILO DE DETECCION DE INTERBLOQUEO: Iniciado.");

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	// Agrego descriptor de comunicacion con plataforma por pipe
	agregar_descriptor(hiloInterbloqueo.fdPipe[0], &master, &max_desc);

	TiempoChequeoDeadlock = configNivelTiempoChequeoDeadlock();
	RecoveryOn = configNivelRecovery();
	hayDeadLock = 0;

	while(!fin) {

		FD_ZERO (&read_fds);
		read_fds = master;
		timeout.tv_sec = 0; /// timeout en segundos
		timeout.tv_usec = TiempoChequeoDeadlock * 1000; //timeout en microsegundos

		if((ret = select(max_desc+1, &read_fds, NULL, NULL, &timeout )) == -1)
		{
			puts("\n\nINTERBLOQUEO: ERROR en select!!\n");

		} else if (ret == 0) {

			// Aca va la logica de interbloqueo
			hayDeadLock = detectarDeadlock();

			log_info(LOGGER, "hayDeadLock: %d  - RecoveryOn: %d \n\n", hayDeadLock, RecoveryOn);
			if (hayDeadLock && RecoveryOn) {
				log_info(LOGGER, "Hay interbloqueo y el recovery esta activado...\n\n");
				recovery(hayDeadLock);
			}

		} else {


			for(i = 0; i <= max_desc; i++)
			{
				if (FD_ISSET(i, &read_fds) && (i == hiloInterbloqueo.fdPipe[0]) ) {
					log_info(LOGGER, "INTERBLOQUEO: Recibo mensaje desde Nivel por Pipe");
					read (hiloInterbloqueo.fdPipe[0], &header, sizeof(header_t));

					log_debug(LOGGER, "INTERBLOQUEO: mensaje recibido '%d'", header.tipo);

					if (header.tipo == FINALIZAR) {
						log_debug(LOGGER, "INTERBLOQUEO: '%d' ES FINALIZAR", header.tipo);
						fin = true;
						//FD_CLR(hiloInterbloqueo.fdPipe[0], &master);
						quitar_descriptor(hiloInterbloqueo.fdPipe[0], &master, &max_desc);
						break;
					}
				}
			}
		}


	}

	log_info(LOGGER, "FINALIZANDO HILO INTERBLOQUEO...\n");

	pthread_exit(NULL);

}


/**
 * 1) Se marca cada proceso que tenga una fila de la matriz de Asignacion completamente a cero
 * 2) Se inicia un vector temporal T asignandole el vector de disponibles
 * 3) Se busca un indice i tal que el proceso i no este marcado actualmente y la fila i-esima de S(olicitud) sea menor o igual a T (disponibles).
 *    Es decir, Se ejecuta Tk = Tk + Aik, para 1 <= k <= m. A continuacion se vuelve al 3 paso.
 */
int32_t detectarDeadlock() {
	log_info(LOGGER, "Incio deteccion de interbloqueo...");
	int32_t hayDeadLock;

	hayDeadLock = 0;

	initMatrices();

	// TODO agregar algoritmo de deteccion de interbloqueo
	// Lleno las matrices y los vectores necesarios.

	t_queue *listaPJEnNivel = clonarListaPersonajesEnNivel();
	t_list *listaPJBloqueados = clonarListaPersonajesBloqueados();
	t_dictionary *recursosxPJ = clonarListaRecursosxPersonaje();
	t_dictionary *dicRecursos = configNivelRecursos();

	totalPersonajes = obtenerCantPersonajesEnNivel();
	totalRecursos = dictionary_size(dicRecursos);

	llenarVecPJEnNivel(listaPJEnNivel);
	llenarRecursos(dicRecursos);
	llenarMatAsignacion(recursosxPJ);
	llenarMatSolicitud(listaPJBloqueados);


	log_info(LOGGER, "\n\n INTERBLOQUEO: totalPersonajes: %d, totalRecursos: %d", totalPersonajes, totalRecursos);
	imprimirMatrices();
	// 1) Se marca cada proceso que tenga una fila de la matriz de Asignacion completamente a cero
	// Tomo listado de personajes en Nivel que no tengan asignado ningun recurso, y lo marco
	marcarPersonajesSinRecursosAsig();

	// 2) Se inicia un vector temporal T asignandole el vector de disponibles
	copiarDisponiblesAT();

	// 3) Se busca un indice i tal que el proceso i no este marcado actualmente
	//    y la fila i-esima de S(olicitud) sea menor o igual a T (disponibles).
	marcarNoBloqueados();

	imprimirMatrices();
	// Existe un interbloqueo si y solo si hay procesos sin marcar al final del algoritmo
	hayDeadLock = contarPersonajesSinMarcar();

	if (hayDeadLock > 0){
		log_info(LOGGER, "HAY INTERBLOQUEO:");
	}

	queue_destroy_and_destroy_elements(listaPJEnNivel, (void*)destruirPersonaje);
	list_destroy_and_destroy_elements(listaPJBloqueados, (void*)destruirPersonaje);
	dictionary_destroy_and_destroy_elements(recursosxPJ, (void*)destruirVecRecursos);
	dictionary_destroy_and_destroy_elements(dicRecursos, (void*)destruirCaja);


	return hayDeadLock;

}


t_personaje* recovery(int32_t cantInterbloqueados) {
	pthread_mutex_lock (&mutexListaPersonajesEnNivel);
	//pthread_mutex_lock (&mutexListaPersonajesBloqueados);
	pthread_mutex_lock (&mutexListaPersonajesMuertosxRecovery);
	log_info(LOGGER, "Incio proceso de recovery deadlock... cantidad Personajes INTERBLOQUEADOS: %d", cantInterbloqueados);
	t_personaje *personaje = NULL, *aux = NULL;
	int32_t totalPersonajesEnNivel = 0;
	int i, j, encontreVictima = 0;

	// TODO agregar logica de recovery
	// 1- Seleccionar la victima (es la primera que entro al nivel)
	// 2- Mover al personaje seleccionado de los listados (deberia estar en bloqueados solamente) y agregarlo al listado MuerteXrecovery.
	// 3- Informar al nivel que hay un personaje muerto (el nivel debe encargarse de informar al personaje correspondiente).

	totalPersonajesEnNivel = queue_size(listaPersonajesEnNivel);

	log_debug(LOGGER, "\n\n totalPersonajesEnNivel: %d \n\nInterbloqueados: \n\n", totalPersonajesEnNivel);
	for (i=0; i < cantInterbloqueados; i++){
		log_debug(LOGGER,"\r     personaje: %c                                                  ", interbloqueados[i]);
	}

	for (i=0; i < totalPersonajesEnNivel; i++) {
		aux = queue_pop(listaPersonajesEnNivel);
		queue_push(listaPersonajesEnNivel, aux);

		log_debug(LOGGER, "RECOVERY %s (%c) esta interbloqueado?", aux->nombre, aux->id);
		if (encontreVictima == 0) {
			for(j=0; j<cantInterbloqueados; j++) {
				log_debug(LOGGER, "RECOVERY %c == %c ", aux->id, interbloqueados[j]);
				if(aux->id == interbloqueados[j]) {
					log_debug(LOGGER, "RECOVERY %c == %c: %d ", aux->id, interbloqueados[j], aux->id == interbloqueados[j]);
					personaje = aux;
					quitarPersonajeBloqueadosNivel(personaje->id);
					queue_push(listaPersonajesMuertosxRecovery, personaje);
					encontreVictima = 1;
					break;
				}
			}
		}
	}

	log_debug(LOGGER, "RECOVERY encontreVictima: %d ", encontreVictima);

	if (encontreVictima == 1) {
		log_debug(LOGGER, "RECOVERY Encontre victima: %s (%c)", personaje->nombre, personaje->id);
		enviarMsjPorPipe(hiloInterbloqueo.fdPipeI2N[1], MUERTE_PERSONAJE_XRECOVERY);
	}

	pthread_mutex_unlock (&mutexListaPersonajesMuertosxRecovery);
	//pthread_mutex_unlock (&mutexListaPersonajesBloqueados);
	pthread_mutex_unlock (&mutexListaPersonajesEnNivel);

	return personaje;
}


int initMatrices() {
	int i,j;

	for (i=0; i < MAXPER; i++) {
		vecPersonajesEnNivel[i][0] = 0;
		vecPersonajesEnNivel[i][1] = 0;
		interbloqueados[i] = '\0';
		for (j=0; j < MAXREC; j++){
			matAsignacion[i][j] = 0;
			matSolicitud[i][j] = 0;
			T[j] = 0;
			vecRecursos[j] = 0;
			vecDisponibles[j] = 0;
			vecRecursosCantTotal[j] = 0;
		}
	}

	return 0;
}

int imprimirMatrices() {
	int i,j;
	char a[50] = {0};
	char r[50] = {0};

	log_debug(LOGGER, "\n\n vecDisponibles \n");
	for (i=0; i < totalPersonajes; i++){
		r[i] = vecRecursos[i];
		log_debug(LOGGER,"\r     recurso: %c disponible: %d                                                  ", vecRecursos[i], vecDisponibles[i]);
	}

	log_debug(LOGGER, "\r                                                                             \n\n");
	log_debug(LOGGER, "\r  copia T de vecDisponibles                                                  \n");
	for (i=0; i < totalPersonajes; i++){
		log_debug(LOGGER,"\r     recurso: %c disponible: %d                                                  ", vecRecursos[i], T[i]);
	}

	log_debug(LOGGER, "\r                                                                             \n\n");
	log_debug(LOGGER, "\r matAsignacion                                                  \n");
	log_debug(LOGGER,"\r     %s                                                  ", r);
	for (i=0; i < totalPersonajes; i++){
		for (j=0; j < totalRecursos; j++){
			a[j] = matAsignacion[i][j]+48;
		}
		log_debug(LOGGER,"\r     %s                                                  ", a);
	}

	log_debug(LOGGER, "\r                                                                             \n\n");
	log_debug(LOGGER, "\r matSolicitud                                                  \n");
	log_debug(LOGGER,"\r     %s                                                  ", r);
	for (i=0; i < totalPersonajes; i++){
		for (j=0; j < totalRecursos; j++){
			//log_debug(LOGGER, "\r     matSolicitud[%d][%d]:  %d \n", i, j,  matSolicitud[i][j]);
			a[j] = matSolicitud[i][j]+48;
		}
		log_debug(LOGGER,"\r     %s                                                  ", a);
	}

	log_debug(LOGGER, "\r                                                                             \n\n");
	log_debug(LOGGER, "\r vecPersonajesEnNivel                                                  \n");
	for (i=0; i < totalPersonajes; i++){
		log_debug(LOGGER,"\r     personaje: %c marca: %d                                                  ", vecPersonajesEnNivel[i][0], vecPersonajesEnNivel[i][1]);
	}

	return 0;
}

int obtenerPosPersonaje(char p) {
	int i;
	for (i = 0; i < totalPersonajes; i++)
		if (vecPersonajesEnNivel[i][0] == p)
			return i;
	return -1;
}


int obtenerPosRecurso(char r) {
	int i;
	for (i = 0; i < totalRecursos; i++)
		if (vecRecursos[i] == r)
			return i;
	return -1;
}


int llenarVecPJEnNivel(t_queue *listaPJEnNivel) {
	int i, total = queue_size(listaPJEnNivel);
	t_personaje *p;

	for (i = 0; i < total; i++) {
		p = queue_pop(listaPJEnNivel);
		queue_push(listaPJEnNivel, p),
		vecPersonajesEnNivel[i][0] = p->id;
		vecPersonajesEnNivel[i][1] = 0;
	}

	return 0;
}

int llenarRecursos(t_dictionary *dicRecursos) {
	int i=0;

	void _fillvec(char *k, t_caja *c) {
		vecRecursos[i] = c->SIMBOLO;
		vecRecursosCantTotal[i] = c->INSTANCIAS;
		vecDisponibles[i] = c->INSTANCIAS;
		i++;
	}

	dictionary_iterator(dicRecursos, (void*)_fillvec);

	return 0;
}



int llenarMatAsignacion(t_dictionary *recursosxPJ ) {
	int p=0, r=0, i;

	void _fillvec(char *pj, t_vecRecursos *vec) {

		p = obtenerPosPersonaje(pj[0]);
		if (p != -1) {
			for (i=0; i < vec->total; i++){
				r = obtenerPosRecurso(vec->recurso[i]);
				if (r != -1){
					matAsignacion[p][r]+=1;
					vecDisponibles[r] -= 1;
				}
			}
		}
	}

	dictionary_iterator(recursosxPJ, (void*)_fillvec);

	return 0;
}


int llenarMatSolicitud(t_list *listaPJBloqueados ) {
	int p=0, r=0;

	void _fillvec(t_personaje *pj) {

		p = obtenerPosPersonaje(pj->id);
		if (p != -1) {
			r = obtenerPosRecurso(pj->recurso);
			if (r != -1)
				matSolicitud[p][r]+=1;

		}
	}

	list_iterate(listaPJBloqueados, (void*)_fillvec);

	return 0;
}


int marcarPersonajesSinRecursosAsig(){
	int i,j, total=0;

	for (i=0; i < totalPersonajes; i++){
		total = 0;
		for (j=0; j < totalRecursos; j++){
			 total +=matAsignacion[i][j];
		}
		if (total == 0){
			vecPersonajesEnNivel[i][1] = 1;
		}

	}

	return 0;
}

int copiarDisponiblesAT(){
	int i;

	for (i=0; i < totalRecursos; i++){
		T[i] = vecDisponibles[i];
	}
	return 0;
}


int marcarNoBloqueados() {
	int i, k, solicitudMIDisp;
	int continuar = true;

	// 3) Se busca un indice i tal que el proceso i no este marcado actualmente
	//    y la fila i-esima de S(olicitud) sea menor o igual a T (disponibles).
	//    Es decir, Sik <= Tk, para 1<=k<=m. Si no se encuentra ninguna fila el algoritmo termina
	while(continuar) {
		continuar = false;

		for (i = 0; i < totalPersonajes; i ++) {

			// Se busca un indice i tal que el proceso i no este marcado actualmente
			if (vecPersonajesEnNivel[i][1] == 0) {
				solicitudMIDisp = 1;

				// y la fila i-esima de S(olicitud) sea menor o igual a T (disponibles).
				// Toda la fila debe cumplir este requerimiento!
				for (k=0; k < totalRecursos; k++) {
					solicitudMIDisp = solicitudMIDisp && (matSolicitud[i][k] <= T[k]);
				}

				if (solicitudMIDisp) {
					// 4) Si se encuentra una fila que lo cumpla, se marca el proceso i
					vecPersonajesEnNivel[i][1] = 1;

					// y se suma la fila correspondiente de la matriz de Asignacion a T
					// Es decir, Se ejecuta Tk = Tk + Aik, para 1 <= k <= m.
					for (k=0; k < totalRecursos; k++) {
						T[k] += matAsignacion[i][k];
					}

					// A continuacion se vuelve al tercer paso
					continuar = true;
					break;
				}
			}
		}
	}

	return 0;
}


int contarPersonajesSinMarcar() {
	int i, cont = 0;
	for (i = 0; i < totalPersonajes; i ++) {
		// Se busca un indice i tal que el proceso i no este marcado actualmente
		if (vecPersonajesEnNivel[i][1] == 0) {
			interbloqueados[cont] = vecPersonajesEnNivel[i][0];
			log_debug(LOGGER, "\n\n\n ----- INTERBLOQUEADO %d: %c\n", cont+1, interbloqueados[cont] );
			cont++;
		}
	}
	return cont;
}



