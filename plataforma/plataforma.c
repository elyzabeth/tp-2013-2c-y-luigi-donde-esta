#include "plataforma.h"


pthread_t idHiloOrquestador;
pthread_t idHilosPlanificador[100];
int32_t cantHilosPlanificador;

void principal();

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

	int id_proceso, i, se_desconecto;

	/************************************ Inicio  **************************************/
	id_proceso = getpid();
	system("clear");
	log_info(LOGGER, "************** Plataforma (PID: %d) ***************\n",id_proceso);


	/****************************** Logica Hilos ****************************************/

	log_info(LOGGER, "Creando hilo orquestador...");
	pthread_create (&idHiloOrquestador, NULL, (void*) orquestador, NULL);


	/***************************** LOGICA PRINCIPAL ********************************/

	header_t header;
	fd_set master;
	fd_set read_fds;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	int max_desc = 0;
	int nuevo_sock;
	int listener;



	/****************************** Creacion Listener ****************************************/

	log_info(LOGGER, "****************** CREACION DEL LISTENER *****************\n");

	//crear_listener(/*puerto*/1500, &listener);
	// Uso puerto seteado en el archivo de configuracion
	crear_listener(PUERTO, &listener);

	agregar_descriptor(listener, &master, &max_desc);

	while(1)
	{

		FD_ZERO (&read_fds);
		read_fds = master;

		if((select(max_desc+1, &read_fds, NULL, NULL, NULL/*&tvDemora*/)) == -1)
		{
			puts("a");
		}

		for(i = 0; i <= max_desc; i++)
		{
			//otrosDescriptor = 1;
			if (FD_ISSET(i, &read_fds) && (i == listener))
			{
				/* nueva conexion */
				log_info(LOGGER, "NUEVA CONEXION");

				aceptar_conexion(&listener, &nuevo_sock);
				//	FD_SET(desc, listaDesc);
				//agregar_descriptor(nuevo_sock, &master, &max_desc);

				recibir_header(nuevo_sock, &header, &master, &se_desconecto);

				switch (header.tipo)
				{
					case NUEVO_PERSONAJE:
						log_info(LOGGER, "NUEVO PERSONAJE");
						nuevoPersonaje(nuevo_sock, &master);
						break;

					case NUEVO_NIVEL:
						log_info(LOGGER, "NUEVO NIVEL");
						nuevoNivel(nuevo_sock, header);
						agregar_descriptor(nuevo_sock, &master, &max_desc);
						break;

					case OTRO:
						break;
				}
			}

			if (FD_ISSET(i, &read_fds) && (i != listener))
			{
				log_debug(LOGGER, "recibo mensaje socket %d", i);
				recibir_header(i, &header, &master, &se_desconecto);
				log_debug(LOGGER, "el tipo de mensaje es: %d\n", header.tipo);

				if(se_desconecto)
				{
					log_info(LOGGER, "Se desconecto el socket %d", i);
					// TODO chequear si se desconecto personaje o nivel y borrarlo de las estructuras
					// si es personaje informar al nivel para que lo borre?
					// plataforma.personajes_en_juego--;

				}

				if ((header.tipo == CONECTAR_NIVEL) && (se_desconecto != 1))
				{
					puts("CONECTAR NIVEL"); //Nunca voy a necesitar este mensaje.

				}

			}
		}
	}

	pthread_join (idHiloOrquestador, NULL); //espera que finalice el hilo orquestador para continuar

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

	log_info(LOGGER, "Envio mensaje de personaje conectado...");

	ret =  enviar(fd, buffer_header, sizeof(header_t));
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

	log_info(LOGGER, "Envio mensaje de personaje conectado...");

	ret = enviar(fd, buffer_header, sizeof(header_t));

	free(buffer_header);

	return ret;
}

void nuevoPersonaje(int fdPersonaje, fd_set *master) {
	t_nivel *nivel;
	header_t header;
	t_personaje *personaje;
	int se_desconecto;

	/**Contesto Mensaje **/

	log_info(LOGGER, "Envio mensaje de PERSONAJE_CONECTADO al personaje...");
	if (enviarMsjPersonajeConectado(fdPersonaje) != EXITO)
	{
		log_error(LOGGER, "Error al enviar header PERSONAJE_CONECTADO\n\n");

	} else {
		// TODO quitar hardcodeo!!
		// recibir informacion del personaje.
		// SI existe el nivel solicitado lo agrego si no, enviar mensaje de nivel inexistente??

		log_debug(LOGGER, "Espero header CONECTAR_NIVEL (%d)...", CONECTAR_NIVEL);
		recibir_header(fdPersonaje, &header, master, &se_desconecto);
		log_debug(LOGGER, "Recibo %d...", header.tipo);

		if (!se_desconecto && header.tipo == CONECTAR_NIVEL) {
			personaje = crearPersonajeVacio();
			log_debug(LOGGER, "Espero recibir estructura personaje (size:%d)...", header.largo_mensaje);
			recibir_personaje(fdPersonaje, personaje, master, &se_desconecto);
			log_debug(LOGGER, "Llego: %s, %c, %s", personaje->nombre, personaje->id, personaje->nivel);

			// Verifico si existe el nivel solicitado
			if(dictionary_has_key(listaNiveles, personaje->nivel)) {
				nivel = dictionary_get(listaNiveles, personaje->nivel);

				log_info(LOGGER, "Agrego personaje a lista de personajes Nuevos.");
				agregarPersonajeNuevo(personaje);
				plataforma.personajes_en_juego++;

				//Informo al planificador correspondiente
				log_info(LOGGER, "Informo al planificador correspondiente.");

				//enviar(nivel->fdPipe[1], buffer_header, sizeof(header_t));
				write (nivel->fdPipe[1], "NUEVOPERSONAJE", 15);
			} else {
				//TODO Que hago si no existe el nivel????
			}
		} else {
			log_info(LOGGER, "Se esperaba mensaje CONECTAR_NIVEL se recibio: %d",header.tipo);
		}

	}
}

void nuevoNivel(int fdNivel, header_t header) {
	//header_t header;
	char *buffer;
	t_nivel *nivel;

	/************************************************/
	buffer = calloc(header.largo_mensaje+1, sizeof(char));
	recibir (fdNivel, buffer, header.largo_mensaje);

	nivel = crearNivel(buffer, fdNivel);

	log_info(LOGGER, "Se conecto el Nivel: %s\n", nivel->nombre);

	/**Contesto Mensaje **/
	if (enviarMsjNivelConectado(fdNivel) != EXITO)
	{
		log_error(LOGGER, "Error al enviar header NIVEL_CONECTADO\n\n");
	} else {
		// TODO levantar hilo Planificador para el nivel
		// pedir/recibir informacion del nivel.
		log_info(LOGGER, "Levanto Hilo Planificador para Nivel '%s'", nivel->nombre);
		pthread_create(&idHilosPlanificador[cantHilosPlanificador++], NULL, (void*)planificador, nivel);
		// Agrego el nivel al diccionario de nieveles
		dictionary_put(listaNiveles, nivel->nombre, nivel);
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
	listaNiveles = dictionary_create();
	pthread_mutex_init (&mutexListaPersonajesNuevos, NULL);
	pthread_mutex_init (&mutexListaPersonajesEnJuego, NULL);
	cantHilosPlanificador = 0;
}

/**
 * @NAME: finalizarPlataforma
 * @DESC: Finaliza todas las variables y estructuras que fueron creadas para el proceso plataforma
 */
void finalizarPlataforma() {
	log_info(LOGGER, "FINALIZANDO PLATAFORMA");
	// Libero listas
	list_destroy_and_destroy_elements(listaPersonajesNuevos, (void*)free);
	list_destroy_and_destroy_elements(listaPersonajesEnJuego, (void*)free);
	dictionary_destroy_and_destroy_elements(listaNiveles, (void*)free);

	// Libero semaforos
	pthread_mutex_destroy(&mutexListaPersonajesNuevos);
	pthread_mutex_destroy(&mutexListaPersonajesEnJuego);

	// Libero estructuras de configuracion
	destruirConfigPlataforma();
	// Libero logger
	log_destroy(LOGGER);

	// Libero a Willy!
	// free (Willy);
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
