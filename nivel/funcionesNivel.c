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


void moverPersonajeABloqueados(char simboloPersonaje) {
	t_personaje *personaje;

	personaje = quitarPersonajeEnJuegoNivel(simboloPersonaje);

	if (personaje != NULL)
		agregarPersonajeABloqueadosNivel(personaje);

}

/**
 * @NAME: posicionDentroDeLosLimites
 * @DESC: Valida que (x, y) sea mayor a (0, 0) y menor a los limites de la pantalla (MAXCOLS, MAXROWS)
 */
bool posicionDentroDeLosLimites (int32_t x, int32_t y) {

	if(x < 0 || x > MAXCOLS || y < 0 || y > MAXROWS) {
		return false;
	}

	return true;
}

/**
 * @NAME: validarPosicionCaja
 * @DESC: Valida que la caja de recursos que se va a agregar este dentro de los limites.
 */
void validarPosicionCaja(char s, int32_t x, int32_t y) {

	//if(x < 0 || x > MAXCOLS || y < 0 || y > MAXROWS) {
	if ( !posicionDentroDeLosLimites(x, y) ) {
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
		enemy = crearHiloEnemigo(idEnemigo);

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
	memset(&hiloInterbloqueo, 0, sizeof(t_hiloInterbloqueo));
	pipe(hiloInterbloqueo.fdPipe);
	pipe(hiloInterbloqueo.fdPipeI2N);

	pthread_mutex_init (&mutexLockGlobalGUI, NULL);
	pthread_mutex_init (&mutexListaPersonajesJugando, NULL);
	pthread_mutex_init (&mutexListaPersonajesBloqueados, NULL);
	pthread_mutex_init (&mutexListaPersonajesFinalizados, NULL);
	pthread_mutex_init (&mutexListaPersonajesEnNivel, NULL);
	pthread_mutex_init (&mutexListaRecursos, NULL);
	pthread_mutex_init (&mutexRecursosxPersonajes, NULL);

	// inicializo listas
	listaEnemigos = list_create();
	listaPersonajesEnJuego = list_create();
	listaPersonajesBloqueados = list_create();
	listaPersonajesFinalizados = list_create();
	listaPersonajesEnNivel = queue_create();
	recursosxPersonajes = dictionary_create();

	listaRecursos = configNivelRecursos();

	// inicializo inotify
	notifyFD = crearNotifyFD();
	watchDescriptor = inotify_add_watch(notifyFD, PATH_CONFIG_NIVEL, IN_MODIFY);

	//inicializar NIVEL-GUI
	inicializarNivelGui();

}


void finalizarPersonajeNivel(t_personaje *personaje) {
	t_personaje *p;
	t_vecRecursos *vec;
	t_caja* caja;
	int i;

	// LIBERO LOS RECURSOS ASIGNADOS AL PERSONAJE
	log_info(LOGGER, "%s Liberando recursos del personaje %s", NOMBRENIVEL, personaje->nombre);
	vec = removerRecursoxPersonaje(personaje);
	for(i = 0; i < vec->total; i++) {
		gui_sumarRecurso(vec->recurso[i]);
		caja = obtenerRecurso(vec->recurso[i]);
		caja->INSTANCIAS++;
	}

	// QUITO AL PERSONAJE DE LISTADOS DINAMICOS
	p = quitarPersonajeEnJuegoNivel(personaje->id);
	if (NULL == p)
		p = quitarPersonajeBloqueadosNivel(personaje->id);

	if (p != NULL)
		agregarPersonajeAFinalizadosNivel(p);

	// BORRO al personaje del listado GUIITEMS
	gui_borrarItem(personaje->id);
}

/**
 * @NAME: finalizarHilosEnemigos
 * @DESC: Finaliza los hilos creados para cada enemigo enviandoles por pipe el mensaje FINALIZAR
 */
void finalizarHilosEnemigos() {
	int i = 0;
	int32_t cantEnemigos = configNivelEnemigos();

	log_info(LOGGER, "%s: FINALIZANDO HILOS ENEMIGOS", NOMBRENIVEL);

	void _finalizar_hilo(t_hiloEnemigo *enemy) {
		log_debug(LOGGER, "%s: %d/%d) Envio mensaje de FINALIZAR a Enemigo '%c' (%u)", NOMBRENIVEL, ++i, cantEnemigos, enemy->enemigo.id, enemy->tid);

		enviarMsjPorPipe(enemy->fdPipe[1], FINALIZAR);
		pthread_join(enemy->tid, NULL);
		//sleep(1);
	}

	list_iterate(listaEnemigos, (void*)_finalizar_hilo);

}

void finalizarNivel () {

	log_info(LOGGER, "FINALIZANDO NIVEL-GUI '%s'", NOMBRENIVEL);

	// finalizo hilos enemigos antes de finalizar la GUI si no da error.
	finalizarHilosEnemigos();

	// Libero / finalizo NIVEL-GUI
	nivel_gui_terminar();

	log_info(LOGGER, "FINALIZANDO NIVEL '%s'", NOMBRENIVEL);

	// finalizo hilo Interbloqueo
	enviarMsjAInterbloqueo(FINALIZAR);
	close(hiloInterbloqueo.fdPipe[0]);
	close(hiloInterbloqueo.fdPipe[1]);
	close(hiloInterbloqueo.fdPipeI2N[0]);
	close(hiloInterbloqueo.fdPipeI2N[1]);


	// Libero listas dinamicas
	list_destroy_and_destroy_elements(GUIITEMS, (void*)free);
	list_destroy_and_destroy_elements(listaEnemigos, (void*)destruirHiloEnemigo);
	list_destroy_and_destroy_elements(listaPersonajesEnJuego, (void*)destruirPersonaje);
	list_destroy_and_destroy_elements(listaPersonajesBloqueados, (void*)destruirPersonaje);
	list_destroy_and_destroy_elements(listaPersonajesFinalizados, (void*)destruirPersonaje);
	queue_destroy_and_destroy_elements(listaPersonajesEnNivel, (void*)destruirPersonaje);
	dictionary_destroy_and_destroy_elements(listaRecursos, (void*)destruirCaja);
	dictionary_destroy_and_destroy_elements(recursosxPersonajes, (void*)destruirVecRecursos);

	// Finalizo inotify
	inotify_rm_watch(notifyFD, watchDescriptor);
	close(notifyFD);

	// libero semaforos
	pthread_mutex_destroy(&mutexLockGlobalGUI);
	pthread_mutex_destroy(&mutexListaPersonajesJugando);
	pthread_mutex_destroy(&mutexListaPersonajesBloqueados);
	pthread_mutex_destroy(&mutexListaPersonajesFinalizados);
	pthread_mutex_destroy(&mutexListaPersonajesEnNivel);
	pthread_mutex_destroy(&mutexListaRecursos);
	pthread_mutex_destroy(&mutexRecursosxPersonajes);

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


int agregarFDPipeEscuchaEnemigo(fd_set *listaDesc, int *maxDesc) {

	void _add_enemy_fd(t_hiloEnemigo *enemy) {
		// Agrego descriptor del Pipe con Nivel.
		agregar_descriptor(enemy->fdPipeE2N[0], listaDesc, maxDesc);
	}

	list_iterate(listaEnemigos, (void*)_add_enemy_fd);

	return EXITO;
}


/*
 * @NAME: signal_callback_handler
 * @DESC: Define la funcion a llamar cuando una señal es enviada al proceso
 * ej: ctrl-c (SIGINT)
 */
void signal_callback_handler(int signum)
{
	log_info(LOGGER, "%s INTERRUPCION POR SEÑAL: %d = %s \n", NOMBRENIVEL, signum, strsignal(signum));

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

t_vecRecursos* crearVecRecursos() {
	t_vecRecursos *vec = calloc(1, sizeof(t_vecRecursos));
	vec->total = 0;
	return vec;
}

void destruirVecRecursos(t_vecRecursos *vecRecursos) {
	free(vecRecursos);
}

void agregarRecursoVec(t_vecRecursos *vecRecursos, char recurso) {
	vecRecursos->recurso[vecRecursos->total++] = recurso;
}


/////////////////////////////////////////////////////////////////////////

int enviarMsjPorPipe (int32_t fdPipe, char msj) {
	int ret;
	header_t header;
	char* buffer_header = calloc(1,sizeof(header_t));

	initHeader(&header);
	header.tipo = msj;
	header.largo_mensaje=0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	ret =  write(fdPipe, buffer_header, sizeof(header_t));

	free(buffer_header);

	return ret;
}


int enviarMsjAInterbloqueo (char msj) {
	int ret;

	log_info(LOGGER, "%s Enviando mensaje al hilo de interbloqueo.", NOMBRENIVEL);

	ret =  enviarMsjPorPipe(hiloInterbloqueo.fdPipe[1], msj);
	pthread_join(hiloInterbloqueo.tid, NULL);

	return ret;
}

int enviarMSJNuevoNivel(int sock) {
	header_t header;
	t_nivel nivel;
	char* buffer_payload;

	initHeader(&header);
	header.tipo = NUEVO_NIVEL;
	header.largo_mensaje = sizeof(t_nivel);

	log_info(LOGGER, "%s enviarMSJNuevoNivel: sizeof(header): %d, largo mensaje: %d \n", NOMBRENIVEL, sizeof(header), header.largo_mensaje);

	if (enviar_header(sock, &header) != EXITO)
	{
		log_error(LOGGER,"%s enviarMSJNuevoNivel: Error al enviar header NUEVO_NIVEL\n\n", NOMBRENIVEL);
		return WARNING;
	}

	initNivel(&nivel);
	strcpy(nivel.nombre, configNivelNombre());
	strcpy(nivel.algoritmo, configNivelAlgoritmo());
	nivel.quantum = configNivelQuantum();
	nivel.retardo = configNivelRetardo();

	buffer_payload = calloc(1,sizeof(t_nivel));
	memcpy(buffer_payload, &nivel, sizeof(t_nivel));

	if (enviar(sock, buffer_payload, header.largo_mensaje) != EXITO)
	{
		log_error(LOGGER,"%s enviarMSJNuevoNivel: Error al enviar datos del nivel\n\n", NOMBRENIVEL);
		free(buffer_payload);
		return WARNING;
	}

	free(buffer_payload);

	return EXITO;
}

int enviarMsjCambiosConfiguracion(int sock) {
	int ret;
	header_t header;
	t_nivel nivel;

	initHeader(&header);
	header.tipo = CAMBIOS_CONFIGURACION;
	header.largo_mensaje = sizeof(t_nivel);

	log_info(LOGGER, "%s enviarMsjCambiosConfiguracion: fd:%d, sizeof(header): %d, largo mensaje: %d \n", NOMBRENIVEL, sock, sizeof(header), header.largo_mensaje);

	if ((ret = enviar_header(sock, &header)) != EXITO)
	{
		log_error(LOGGER,"enviarMsjCambiosConfiguracion: Error al enviar header CAMBIOS_CONFIGURACION\n\n");
		return WARNING;
	}

	initNivel(&nivel);
	strcpy(nivel.nombre, configNivelNombre());
	strcpy(nivel.algoritmo, configNivelAlgoritmo());
	nivel.quantum = configNivelQuantum();
	nivel.retardo = configNivelRetardo();

	if ((ret = enviar_nivel(sock, &nivel)) != EXITO)
	{
		log_error(LOGGER,"%s enviarMsjCambiosConfiguracion: ERROR al enviar datos del nivel con CAMBIOS_CONFIGURACION\n\n", NOMBRENIVEL);
		return ret;
	}

	return ret;
}

int enviarMsjRecursoInexistente (int sock) {
	int ret;
	header_t header;

	initHeader(&header);
	header.tipo = RECURSO_INEXISTENTE;
	header.largo_mensaje = 0;

	log_info(LOGGER, "%s enviarMsjRecursoInexistente: fd:%d, sizeof(header): %d, largo mensaje: %d \n", NOMBRENIVEL, sock, sizeof(header), header.largo_mensaje);

	if ((ret = enviar_header(sock, &header)) != EXITO)
	{
		log_error(LOGGER,"%s enviarMsjRecursoInexistente: Error al enviar header RECURSO_INEXISTENTE\n\n", NOMBRENIVEL);
		return WARNING;
	}

	return ret;
}


int tratarNuevoPersonaje(int sock, header_t header, fd_set *master) {
	int ret, se_desconecto;
	t_personaje personaje;
	t_vecRecursos *vec;

	// Si llega un mensaje de NUEVO_PERSONAJE luego espero recibir un t_personaje
	if ((ret=recibir_personaje(sock, &personaje, master, &se_desconecto)) != EXITO)
	{
		log_error(LOGGER,"%s tratarNuevoPersonaje: ERROR al recibir payload t_personaje en NUEVO_PERSONAJE\n\n", NOMBRENIVEL);
		// TODO cancelo o solo retorno??
		return ret;
	}

	log_debug(LOGGER,"%s tratarNuevoPersonaje: Llego: %s, %c, recurso '%c' \n\n", NOMBRENIVEL, personaje.nombre, personaje.id, personaje.recurso);

	// TODO agregar personaje a lista de personajes en juego
	// y a la lista GUIITEMS para graficarlo.
	agregarPersonajeEnJuegoNivel(crearPersonajeDesdePersonaje(personaje));
	gui_crearPersonaje(personaje.id, personaje.posActual.x, personaje.posActual.y);
	vec = crearVecRecursos();
	agregarRecursoxPersonaje(&personaje, vec);

	return ret;
}

int tratarSolicitudUbicacion(int sock, header_t header, fd_set *master) {
	int ret, se_desconecto;
	t_personaje personaje;
	t_caja *recurso;
	t_caja caja;
	//t_vecRecursos *vec;

	// Si llega un mensaje de SOLICITUD_UBICACION luego espero recibir un t_personaje
	if ((ret=recibir_personaje(sock, &personaje, master, &se_desconecto)) != EXITO)
	{
		log_error(LOGGER,"%s tratarSolicitudUbicacion: ERROR al recibir payload t_personaje en SOLICITUD_UBICACION\n\n", NOMBRENIVEL);
		// TODO cancelo o solo retorno??
		return ret;
	}

	log_debug(LOGGER,"%s tratarSolicitudUbicacion: Llego: %s, %c, recurso '%c' \n\n", NOMBRENIVEL, personaje.nombre, personaje.id, personaje.recurso);
	recurso = obtenerRecurso(personaje.recurso);

	if (recurso == NULL) {
		log_error(LOGGER, "%s tratarSolicitudUbicacion: obtenerRecurso('%c') devuelve NULL!! ", NOMBRENIVEL, personaje.recurso);
		enviarMsjRecursoInexistente(sock);
		return WARNING;
	}

	log_debug(LOGGER, "%s Recurso: %s %s '%c' (%d,%d) = %d", NOMBRENIVEL, recurso->NOMBRE, recurso->RECURSO, recurso->SIMBOLO, recurso->POSX, recurso->POSY, recurso->INSTANCIAS);

	initCaja(&caja);
	caja = *recurso;

	// Envio caja con ubicacion al planificador
	initHeader(&header);
	header.tipo = UBICACION_RECURSO;
	header.largo_mensaje = sizeof(t_caja);

	if ((ret = enviar_header(sock, &header)) != EXITO) {
		log_error(LOGGER,"%s tratarSolicitudUbicacion: ERROR al enviar header UBICACION_RECURSO\n\n", NOMBRENIVEL);
		return ret;
	}

	if ((ret = enviar_caja(sock, &caja)) != EXITO) {
		log_error(LOGGER,"%s tratarSolicitudUbicacion: ERROR al enviar t_caja de UBICACION_RECURSO\n\n", NOMBRENIVEL);
		return ret;
	}

	// MUEVO ESTO A LA FUNCION tratarNuevoPersonaje
//	// agregar personaje a lista de personajes en juego
//	// y a la lista GUIITEMS para graficarlo.
//	agregarPersonajeEnJuegoNivel(crearPersonajeDesdePersonaje(personaje));
//	gui_crearPersonaje(personaje.id, personaje.posActual.x, personaje.posActual.y);
//	vec = crearVecRecursos();
//	agregarRecursoxPersonaje(&personaje, vec);

	return ret;
}

int tratarSolicitudRecurso(int sock, header_t header, fd_set *master) {
	int ret, se_desconecto;
	t_personaje personaje;
	t_caja *recurso;
	t_caja caja;

	// Si llega un mensaje de SOLICITUD_RECURSO luego espero recibir un t_personaje
	if ((ret=recibir_personaje(sock, &personaje, master, &se_desconecto)) != EXITO)
	{
		log_error(LOGGER,"%s tratarSolicitudRecurso: ERROR al recibir payload t_personaje en SOLICITUD_RECURSO\n\n", NOMBRENIVEL);
		// TODO cancelo o solo retorno??
		return ret;
	}

	log_debug(LOGGER,"%s tratarSolicitudRecurso: Llego: %s, %c, recurso '%c' \n\n", NOMBRENIVEL, personaje.nombre, personaje.id, personaje.recurso);
	recurso = obtenerRecurso(personaje.recurso);

	initHeader(&header);
	if (recurso->INSTANCIAS > 0) {
		header.tipo = RECURSO_CONCEDIDO;
		header.largo_mensaje = sizeof(t_caja);

	} else {
		header.tipo = RECURSO_DENEGADO;
		header.largo_mensaje = 0;
		moverPersonajeABloqueados(personaje.id);
	}

	// Envio mensaje RECURSO_CONCEDIDO o RECURSO_DENEGADO al planificador
	if ((ret = enviar_header(sock, &header)) != EXITO) {
		log_error(LOGGER,"%s tratarSolicitudRecurso: ERROR al enviar header RECURSO_CONCEDIDO/RECURSO_DENEGADO \n\n", NOMBRENIVEL);
		return ret;
	}

	if (header.tipo == RECURSO_CONCEDIDO) {
		//Envio recurso al planificador
		initCaja(&caja);
		caja = *recurso;
		if ((ret = enviar_caja(sock, &caja)) != EXITO) {
			log_error(LOGGER,"%s tratarSolicitudRecurso: ERROR al enviar t_caja de RECURSO_CONCEDIDO\n\n", NOMBRENIVEL);
			return ret;
		}

		// Resto 1 instancia del recurso
		recurso->INSTANCIAS--;
		incrementarRecursoxPersonaje(&personaje, personaje.recurso);
		gui_restarRecurso(recurso->SIMBOLO);
		gui_dibujar();
	}

	return ret;
}

int tratarMovimientoRealizado(int sock, header_t header, fd_set *master) {
	int ret, se_desconecto;
	t_personaje personaje;

	// Si llega un mensaje de MOVIMIENTO_REALIZADO luego espero recibir un t_personaje
	if ((ret=recibir_personaje(sock, &personaje, master, &se_desconecto)) != EXITO)
	{
		log_error(LOGGER,"%s tratarMovimientoRealizado: ERROR al recibir payload t_personaje en MOVIMIENTO_REALIZADO\n\n", NOMBRENIVEL);
		// TODO cancelo o solo retorno??
		return ret;
	}

	log_debug(LOGGER, "%s Movimiento realizado por %s '%c': (%d, %d)", NOMBRENIVEL, personaje.nombre, personaje.id, personaje.posActual.x, personaje.posActual.y);
	log_debug(LOGGER, "%s GUIITEMS': %d", NOMBRENIVEL, list_size(GUIITEMS));

	gui_moverPersonaje(personaje.id, personaje.posActual.x, personaje.posActual.y);

	return ret;
}


int tratarPlanNivelFinalizado(int sock, header_t header, fd_set *master) {
	int ret, se_desconecto;
	t_personaje personaje;

	// Si llega un mensaje de PLAN_NIVEL_FINALIZADO luego espero recibir un t_personaje
	if ((ret=recibir_personaje(sock, &personaje, master, &se_desconecto)) != EXITO)
	{
		log_error(LOGGER,"%s tratarPlanNivelFinalizado: ERROR al recibir payload t_personaje en PLAN_NIVEL_FINALIZADO\n\n", NOMBRENIVEL);
		// TODO cancelo o solo retorno??
		return ret;
	}

	finalizarPersonajeNivel(&personaje);

	return ret;
}


int tratarMuertePersonaje(int sock, header_t header, fd_set *master) {
	int ret, se_desconecto;
	t_personaje personaje;

	// Si llega un mensaje de MUERTE_PERSONAJE luego espero recibir un t_personaje
	if ((ret=recibir_personaje(sock, &personaje, master, &se_desconecto)) != EXITO)
	{
		log_error(LOGGER,"%s tratarMuertePersonaje: ERROR al recibir payload t_personaje en MUERTE_PERSONAJE \n\n", NOMBRENIVEL);
		return ret;
	}

	//gui_moverPersonaje(personaje.id, personaje.posActual.x, personaje.posActual.y);
	finalizarPersonajeNivel(&personaje);

	return ret;
}

int tratarMuertePersonajexEnemigo (int sock, header_t header, fd_set *master) {
	int ret, se_desconecto;
	t_personaje personaje;


	// Si llega un mensaje de MUERTE_PERSONAJE luego espero recibir un t_personaje
	if ((ret=recibir_personaje(sock, &personaje, master, &se_desconecto)) != EXITO)
	{
		log_error(LOGGER,"%s tratarMuertePersonaje: ERROR al recibir payload t_personaje en MUERTE_PERSONAJE \n\n", NOMBRENIVEL);
		return ret;
	}

	//gui_moverPersonaje(personaje.id, personaje.posActual.x, personaje.posActual.y);
	finalizarPersonajeNivel(&personaje);

	return ret;
}
