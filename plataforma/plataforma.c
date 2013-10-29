#include "plataforma.h"



int main(int argc, char *argv[]) {

	signal(SIGINT, plat_signal_callback_handler);
	signal(SIGQUIT, plat_signal_callback_handler);
	signal(SIGUSR1, plat_signal_callback_handler);
	signal(SIGTERM, plat_signal_callback_handler);

	/********************* Carga de Parametros desde archivo y seteos ***************************/
	inicializarPlataforma();

	principal();

	return EXIT_SUCCESS;
}


/**
 * Logica central de Plataforma
 */
void principal() {

	int id_proceso;

	/************************************ Inicio  **************************************/
	id_proceso = getpid();
	system("clear");
	log_info(LOGGER, "************** Plataforma (PID: %d) ***************\n",id_proceso);


	/****************************** Logica Hilos ****************************************/

	log_info(LOGGER, "Creando hilo orquestador...");
	pthread_create (&hiloOrquestador.tid, NULL, (void*) orquestador, (void*)&hiloOrquestador);

	pthread_join (hiloOrquestador.tid, NULL); //espera que finalice el hilo orquestador para continuar

	return;
}

int enviarMsjPersonajeConectado (int fd) {
	int ret;
	header_t header;
	char* buffer_header = malloc(sizeof(header_t));

	memset(&header, '\0', sizeof(header_t));
	header.tipo=PERSONAJE_CONECTADO;
	header.largo_mensaje=0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	log_info(LOGGER, "Envio mensaje de personaje conectado (fd: %d)...", fd);

	ret =  enviar(fd, buffer_header, sizeof(header_t));
	free(buffer_header);

	return ret;
}

int enviarMsjAPlanificador (t_planificador *planner, char msj) {
	int ret;
	header_t header;
	char* buffer_header = malloc(sizeof(header_t));

	memset(&header, '\0', sizeof(header_t));
	header.tipo = msj;
	header.largo_mensaje=0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	log_info(LOGGER, "Enviando mensaje al planificador (%s).......", planner->nivel.nombre);

	ret =  write(planner->fdPipe[1], buffer_header, sizeof(header_t));

	free(buffer_header);

	return ret;
}

int enviarMsjAOrquestador (char msj) {
	int ret;
	header_t header;
	char* buffer_header = malloc(sizeof(header_t));

	memset(&header, '\0', sizeof(header_t));
	header.tipo = msj;
	header.largo_mensaje=0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	log_info(LOGGER, "Enviando mensaje al orquestador.");

	ret =  write(hiloOrquestador.fdPipe[1], buffer_header, sizeof(header_t));
	pthread_join(hiloOrquestador.tid, NULL);

	free(buffer_header);

	return ret;
}

int enviarMsjNivelConectado (int fd) {
	header_t header;
	char* buffer_header = malloc(sizeof(header_t));
	int ret;

	memset(&header, '\0', sizeof(header_t));
	header.tipo=NIVEL_CONECTADO;
	header.largo_mensaje=0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	log_info(LOGGER, "Envio mensaje de nivel conectado al nivel (fd: %d)...", fd);

	ret = enviar(fd, buffer_header, sizeof(header_t));

	free(buffer_header);

	return ret;
}

void nuevoPersonaje(int fdPersonaje, fd_set *master, int *max_desc) {
	t_planificador *planner;
	header_t header;
	t_personaje *personaje;
	int se_desconecto;

	/**Contesto Mensaje **/

	log_info(LOGGER, "Envio mensaje de PERSONAJE_CONECTADO al personaje...");
	if (enviarMsjPersonajeConectado(fdPersonaje) != EXITO)
	{
		log_error(LOGGER, "Error al enviar header PERSONAJE_CONECTADO\n\n");

	} else {

		// recibir informacion del personaje.
		// SI existe el nivel solicitado lo agrego si no, enviar mensaje de nivel inexistente??

		log_debug(LOGGER, "Espero header CONECTAR_NIVEL (%d)...", CONECTAR_NIVEL);
		recibir_header(fdPersonaje, &header, master, &se_desconecto);
		log_debug(LOGGER, "Recibo %d...", header.tipo);

		if (se_desconecto) {
			log_info(LOGGER, "El personaje se desconecto");
			//FD_CLR(fdPersonaje, master);
			quitar_descriptor(fdPersonaje, master, max_desc);
		}

		if (!se_desconecto && header.tipo == CONECTAR_NIVEL) {
			personaje = crearPersonajeVacio();

			log_debug(LOGGER, "Espero recibir estructura personaje (size:%d)...", header.largo_mensaje);
			recibir_personaje(fdPersonaje, personaje, master, &se_desconecto);
			log_debug(LOGGER, "Llego: %s, %c, %s", personaje->nombre, personaje->id, personaje->nivel);
			personaje->fd = fdPersonaje;

			// Verifico si existe el nivel solicitado y su estado
			if ( existeNivel(personaje->nivel) && (obtenerEstadoNivel(personaje->nivel) == CORRIENDO)) {

				planner = obtenerNivel(personaje->nivel);

				log_info(LOGGER, "Agrego personaje '%s' ('%c') a lista de personajes Nuevos.", personaje->nombre, personaje->id);
				agregarPersonajeNuevo(personaje);
				plataforma.personajes_en_juego++;

				//Informo al planificador correspondiente
				log_debug(LOGGER, "Informo al planificador correspondiente.");
				enviarMsjAPlanificador(planner, NUEVO_PERSONAJE );

			} else {
				//TODO Que hago si no existe el nivel????
				log_info(LOGGER, "El personaje '%s' ('%c') solicita Nivel inexistente (%s).", personaje->nombre, personaje->id, personaje->nivel);
			}
		} else {
			log_info(LOGGER, "Se esperaba mensaje CONECTAR_NIVEL se recibio: %d",header.tipo);
		}

	}

}

void nuevoNivel(int fdNivel, header_t header) {
	char *buffer;
	t_nivel nivel;
	t_planificador *planner;

	/************************************************/
	buffer = calloc(1, header.largo_mensaje);
	recibir (fdNivel, buffer, header.largo_mensaje);
	//memset(&nivel, '\0', sizeof(t_nivel));
	initNivel(&nivel);
	memcpy(&nivel, buffer, sizeof(t_nivel));

	nivel.fdSocket = fdNivel;
	planner = crearPlanificador(nivel);

	log_info(LOGGER, "Se conecto el Nivel: %s\n", nivel.nombre);

	// PRIMERO QUITO NIVELES EN ESTADO FINALIZADO
	eliminarNivelesFinalizados();
	if (!existeNivel(nivel.nombre)) {

		// Agrego el nivel al diccionario de niveles
		agregarAListaNiveles(planner);

		// levantar hilo Planificador para el nivel
		log_info(LOGGER, "Levanto Hilo Planificador para Nivel '%s'", nivel.nombre);
		pthread_create(&planner->tid, NULL, (void*)planificador, planner);

		/**Contesto Mensaje **/
		if (enviarMsjNivelConectado(fdNivel) != EXITO) {
			log_error(LOGGER, "Error al enviar header NIVEL_CONECTADO\n\n");
		}

	} else {
		log_error(LOGGER, "El Nivel '%s' YA EXISTE EN EL SISTEMA!", nivel.nombre);
		// TODO ENVIAR MENSAJE AL NIVEL
	}

	free(buffer);
}


/**
 * @NAME: inicializarPlataforma
 * @DESC: Inicializa todas las variables y estructuras necesarias para el proceso plataforma
 */
void inicializarPlataforma () {
	levantarArchivoConfiguracionPlataforma();
	// TODO agregar inicializaciones necesarias
	LOGGER = log_create(configPlatLogPath(), "plataforma", configPlatLogConsola(), configPlatLogNivel() );
	log_info(LOGGER, "INICIALIZANDO Plataforma en el puerto: '%d' ", configPlatPuerto());

	PUERTO = configPlatPuerto();
	plataforma.personajes_en_juego=0;

	listaPersonajesNuevos = list_create();
	listaPersonajesEnJuego = list_create();
	listaPersonajesFinAnormal = list_create();
	listaNiveles = dictionary_create();
	pthread_mutex_init (&mutexListaPersonajesNuevos, NULL);
	pthread_mutex_init (&mutexListaPersonajesEnJuego, NULL);
	pthread_mutex_init (&mutexListaPersonajesFinAnormal, NULL);
	pthread_mutex_init (&mutexListaNiveles, NULL);

	pipe(hiloOrquestador.fdPipe);
}


/**
 * @NAME: finalizarPlataforma
 * @DESC: Finaliza todas las variables y estructuras que fueron creadas para el proceso plataforma
 */
void finalizarPlataforma() {
	log_info(LOGGER, "FINALIZANDO PLATAFORMA");

	// Bajo los hilos
	matarHilos();

	// Libero listas
	list_destroy_and_destroy_elements(listaPersonajesNuevos, (void*)destruirPersonaje);
	list_destroy_and_destroy_elements(listaPersonajesEnJuego, (void*)destruirPersonaje);
	list_destroy_and_destroy_elements(listaPersonajesFinAnormal, (void*)destruirPersonaje);
	dictionary_destroy_and_destroy_elements(listaNiveles, (void*)destruirPlanificador);

	// Libero semaforos
	pthread_mutex_destroy(&mutexListaPersonajesNuevos);
	pthread_mutex_destroy(&mutexListaPersonajesEnJuego);
	pthread_mutex_destroy(&mutexListaPersonajesFinAnormal);
	pthread_mutex_destroy(&mutexListaNiveles);

	close(hiloOrquestador.fdPipe[0]);
	close(hiloOrquestador.fdPipe[1]);

	// Libero estructuras de configuracion
	destruirConfigPlataforma();

	// Libero logger
	log_destroy(LOGGER);


	// Libero a Willy!
	// free (Willy);
}

void matarHilos() {

	// Finalizo hilo orquetador
	enviarMsjAOrquestador(FINALIZAR);
	sleep(1);

	// Finalizo hilos planificadores
	void _finalizar_hilo(char* key, t_planificador *planner){
		enviarMsjAPlanificador(planner, FINALIZAR );
		pthread_join(planner->tid, NULL);
	}
	dictionary_iterator(listaNiveles, (void*)_finalizar_hilo);

}

/*
 * @NAME: plat_signal_callback_handler
 * @DESC: Define la funcion a llamar cuando una señal es enviada al proceso
 * ctrl-c (SIGINT)
 */
void plat_signal_callback_handler(int signum)
{
	log_info(LOGGER, "INTERRUPCION POR SEÑAL: %d = %s \n", signum, strsignal(signum));

	switch(signum) {

	case SIGUSR1: // SIGUSR1=10 ( kill -s USR1 <PID> )
		log_info(LOGGER, " - LLEGO SEÑAL SIGUSR1\n");
		finalizarPlataforma();
		break;
	case SIGTERM: // SIGTERM=15 ( kill <PID>)
		log_info(LOGGER, " - LLEGO SEÑAL SIGTERM\n");
		finalizarPlataforma();
		break;
	case SIGINT: // SIGINT=2 (ctrl-c)
		log_info(LOGGER, " - LLEGO SEÑAL SIGINT\n");
		finalizarPlataforma();
		break;
	case SIGKILL: // SIGKILL=9 ( kill -9 <PID>)
		log_info(LOGGER, " - LLEGO SEÑAL SIGKILL\n");
		finalizarPlataforma();
		break;
	case SIGQUIT: // SIGQUIT=3 (ctrl-4 o kill -s QUIT <PID>)
		log_info(LOGGER, " - LLEGO SEÑAL SIGQUIT\n");
		finalizarPlataforma();
		break;
	}

	// Termino el programa
	exit(signum);
}
