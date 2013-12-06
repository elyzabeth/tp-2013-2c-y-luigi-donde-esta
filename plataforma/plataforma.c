#include "plataforma.h"



int main(int argc, char *argv[]) {

	signal(SIGINT, plat_signal_callback_handler);
	signal(SIGQUIT, plat_signal_callback_handler);
	signal(SIGUSR1, plat_signal_callback_handler);
	signal(SIGUSR2, plat_signal_callback_handler);
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


/**
 * @NAME: enviarMsjAPlanificador
 * @DESC: Envia un mensaje al hilo planificador a traves de pipe.
 * recibe t_planificador y el mensaje a enviar
 */
int enviarMsjAPlanificador (t_planificador *planner, char msj) {
	int ret;
	header_t header;
	char* buffer_header = malloc(sizeof(header_t));

	initHeader(&header);
	header.tipo = msj;
	header.largo_mensaje=0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	log_info(LOGGER, "Enviando mensaje al planificador (%s).......", planner->nivel.nombre);

	ret =  write(planner->fdPipe[1], buffer_header, sizeof(header_t));

	free(buffer_header);

	return ret;
}

/**
 * @NAME: enviarMsjAOrquestador
 * @DESC: Envia un mensaje al hilo orquestador a traves de pipe.
 * recibe el mensaje a enviar.
 */
int enviarMsjAOrquestador (char msj) {
	int ret;
	header_t header;
	char* buffer_header = malloc(sizeof(header_t));

	initHeader(&header);
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
	listaPersonajesFinalizados = list_create();
	listaNiveles = dictionary_create();

	pthread_mutex_init (&mutexListaPersonajesNuevos, NULL);
	pthread_mutex_init (&mutexListaPersonajesEnJuego, NULL);
	pthread_mutex_init (&mutexListaPersonajesFinAnormal, NULL);
	pthread_mutex_init (&mutexListaPersonajesFinalizados, NULL);
	pthread_mutex_init (&mutexListaNiveles, NULL);

	pipe(hiloOrquestador.fdPipe);
}


/**
 * @NAME: finalizarPlataforma
 * @DESC: Finaliza todas las variables y estructuras e hilos que fueron creados para el proceso plataforma
 */
void finalizarPlataforma() {
	log_info(LOGGER, "FINALIZANDO PLATAFORMA");

	// Bajo los hilos
	matarHilos();

	// Libero listas
	list_destroy_and_destroy_elements(listaPersonajesNuevos, (void*)destruirPersonaje);
	list_destroy_and_destroy_elements(listaPersonajesEnJuego, (void*)destruirPersonaje);
	list_destroy_and_destroy_elements(listaPersonajesFinAnormal, (void*)destruirPersonaje);
	list_destroy_and_destroy_elements(listaPersonajesFinalizados, (void*)destruirPersonaje);
	dictionary_destroy_and_destroy_elements(listaNiveles, (void*)destruirPlanificador);

	// Libero semaforos
	pthread_mutex_destroy(&mutexListaPersonajesNuevos);
	pthread_mutex_destroy(&mutexListaPersonajesEnJuego);
	pthread_mutex_destroy(&mutexListaPersonajesFinAnormal);
	pthread_mutex_destroy(&mutexListaPersonajesFinalizados);
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

	// Finalizo hilos planificadores
	void _finalizar_hilo(char* key, t_planificador *planner) {
		if ( planner->estado != FINALIZADO ) {
			enviarMsjAPlanificador(planner, FINALIZAR );
			pthread_join(planner->tid, NULL);
		}
	}
	dictionary_iterator(listaNiveles, (void*)_finalizar_hilo);

	// Finalizo hilo orquestador
	enviarMsjAOrquestador(FINALIZAR);
	sleep(1);
}

void enviarMsjImprimirAPlanificadores(){
	// Envio Msj IMPRIMIR a los planificadores para que impriman el contenido de sus listados
	void _imprimir_listas(char* key, t_planificador *planner) {
		if ( planner->estado != FINALIZADO ) {
			enviarMsjAPlanificador(planner, IMPRIMIR );
			sleep(2);
		}
	}
	dictionary_iterator(listaNiveles, (void*)_imprimir_listas);
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
		imprimirListadoNiveles();
		imprimirListaPersonajesNuevos();
		imprimirListaPersonajesFinAnormal();
		imprimirListaPersonajesEnJuego();
		imprimirListaPersonajesFinalizados();
		//finalizarPlataforma();
		break;
	case SIGUSR2: // SIGUSR2=12 ( kill -s USR2 <PID> )
		log_info(LOGGER, " - LLEGO SEÑAL SIGUSR2\n");
		enviarMsjImprimirAPlanificadores();
		//finalizarPlataforma();
		break;
	case SIGTERM: // SIGTERM=15 ( kill <PID>)
		log_info(LOGGER, " - LLEGO SEÑAL SIGTERM\n");
		finalizarPlataforma();
		// Termino el programa
		exit(signum);
		break;
	case SIGINT: // SIGINT=2 (ctrl-c)
		log_info(LOGGER, " - LLEGO SEÑAL SIGINT\n");
		finalizarPlataforma();
		// Termino el programa
		exit(signum);
		break;
	case SIGKILL: // SIGKILL=9 ( kill -9 <PID>)
		log_info(LOGGER, " - LLEGO SEÑAL SIGKILL\n");
		finalizarPlataforma();
		// Termino el programa
		exit(signum);
		break;
	case SIGQUIT: // SIGQUIT=3 (ctrl-4 o kill -s QUIT <PID>)
		log_info(LOGGER, " - LLEGO SEÑAL SIGQUIT\n");
		finalizarPlataforma();
		// Termino el programa
		exit(signum);
		break;
	}

}
