#include "plataforma.h"


char *buffer_header;


int main(int argc, char *argv[]) {

	int id_proceso, i, se_desconecto;

	signal(SIGINT, plat_signal_callback_handler);
	signal(SIGQUIT, plat_signal_callback_handler);
	signal(SIGUSR1, plat_signal_callback_handler);
	signal(SIGTERM, plat_signal_callback_handler);

	inicializarPlataforma();
/************************************ Inicio  **************************************/

  if(argc != 0) /*cantidad de argumentos para levantar proceso*/
  {
		puts("cantidad Parametros");
  }

	id_proceso = getpid();
	system("clear");
	log_info(LOGGER, "************** Plataforma (PID: %d) ***************\n",id_proceso);

/********************* Carga de Parametros desde archivo y seteos ***************************/

//signal(SIGCHLD, &senialMurioHijo);
//signal(SIGALRM, &senialSuspendido);

/****************************** Logica Hilos ****************************************/

//puts("CREO LOS HILOS ACA");
//pthread_create (&thread_lts, NULL, hiloLts, NULL); //creo e identifico la funcion
//pthread_join (thread_lts, NULL); //espera que finalice el hilo para continuar

/*
printf("HilosIot a crear %d\n", configPP.hilosIot);
		sleep(3);
		for (i = 1 ; i <= configPP.hilosIot; i++)
		{
			sleep(1);
			pthread_t thread_iot[i];

			pthread_create(&thread_iot[i], NULL, hiloIot, NULL);
			//pthread_join(printf("thread_iot%d",i), NULL);
			puts("hilosIOT");
		}
*/
/************************************************/

/***************************** LOGICA PRINCIPAL ********************************/

	header_t header;
	fd_set master;
	fd_set read_fds;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	int max_desc = 0;
	int nuevo_sock;
	int listener;
	//char *buffer_header; //Lo hago global para poder liberarlo desde funcion

	config_plataforma_t plataforma;


/****************************** Creacion Listener ****************************************/

	log_info(LOGGER, "****************** CREACION DEL LISTENER *****************\n");

  //crear_listener(/*puerto*/1500, &listener);
  // Uso puerto seteado en el archivo de configuracion
  crear_listener(/*puerto*/PUERTO, &listener);


  agregar_descriptor(listener, &master, &max_desc);


  //config_plataforma.cantidad_conexiones=0;

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
				agregar_descriptor(nuevo_sock, &master, &max_desc);

				recibir_header(nuevo_sock, &header, &master, &se_desconecto);

				switch (header.tipo)
				{
					case NUEVO_PERSONAJE:
						plataforma.cantidad_conexiones++;
						/**Contesto Mensaje **/
						header.tipo=PERSONAJE_CONECTADO;
						header.largo_mensaje=0;
						//buffer_header = malloc(sizeof(header_t)); /* hace falta remalloc??*/ Si es por unica vez lo muevo al iniciador
						memset(buffer_header, '\0', sizeof(header_t));
						//memset(header, '\0', sizeof(header_t)); // ????

						memcpy(buffer_header, &header, sizeof(header_t)/*TAMHEADER*/);
						if (enviar(nuevo_sock, buffer_header, sizeof(header_t)) != EXITO)
						  {
							log_error(LOGGER, "Error al enviar header NUEVO PERSONAJE\n\n");
							return WARNING;
						  }

					break;

					case OTRO:
					break;

				}
			}
			if (FD_ISSET(i, &read_fds) && (i != listener))
			{
				log_debug(LOGGER, "recibo mensaje");
				recibir_header(i, &header, &master, &se_desconecto);
				log_debug(LOGGER, "el tipo de mensaje es: %d\n", header.tipo);

				if(se_desconecto)
				{
					log_info(LOGGER, "Se desconecto el socket %d", i);
					plataforma.cantidad_conexiones--;
					// TODO informar al nivel para que lo borre?
				}

				if ((header.tipo == CONECTAR_NIVEL) && (se_desconecto != 1))
				{
						puts("CONECTAR NIVEL"); //Nunca voy a necesitar este mensaje.

				}

			}
		}
	}



	return 0;



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
	buffer_header = malloc(sizeof(header_t));
}

/**
 * @NAME: finalizarPlataforma
 * @DESC: Finaliza todas las variables y estructuras que fueron creadas para el proceso plataforma
 */
void finalizarPlataforma() {
	log_info(LOGGER, "FINALIZANDO PLATAFORMA");
	free(buffer_header);
	destruirConfigPlataforma();
	log_destroy(LOGGER);
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
