#include "plataforma.h"

t_log* LOGGER;

int inicializar();


int main(int argc, char *argv[])

{

int id_proceso, i, se_desconecto;

/************************************ Inicio  **************************************/

  if(argc != 0) /*cantidad de argumentos para levantar proceso*/
  {
		puts("cantidad Parametros");
  }

	id_proceso = getpid();
	system("clear");
	printf("************** plataforma (PID: %d) ***************\n",id_proceso);

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
	char *buffer_header;

	config_plataforma_t plataforma;


/****************************** Creacion Listener ****************************************/

  printf("****************** CREACION DEL LISTENER *****************\n");

  crear_listener(/*puerto*/1500, &listener);

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
				puts("NUEVA CONEXION");

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
						buffer_header = malloc(sizeof(header_t)); /* hace falta remalloc??*/
						memset(buffer_header, '\0', sizeof(header_t));
						//memset(header, '\0', sizeof(header_t)); // ????

						memcpy(buffer_header, &header, sizeof(header_t)/*TAMHEADER*/);
						if (enviar(nuevo_sock, buffer_header, sizeof(header_t)) != EXITO)
						  {
							printf("Error al enviar header NUEVO PERSONAJE\n\n");
							return WARNING;
						  }

					break;

					case OTRO:
					break;

				}
			}
			if (FD_ISSET(i, &read_fds) && (i != listener))
			{
				puts("recibo mensaje");
				recibir_header(i, &header, &master, &se_desconecto);
				printf("el tipo de mensaje es: %d\n", header.tipo);

				if(se_desconecto)
				{
					puts("se desconecto");
					plataforma.cantidad_conexiones--;

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
 * @NAME: inicializar
 * @DESC: Inicializa todas las variables y estructuras necesarias para el plataforma
 */
int inicializar() {
	levantarArchivoConfiguracionPlataforma();
	// TODO agregar inicializaciones necesarias
	//LOGGER = log_create(configPlataformaLogPath(), "plataforma", configPlataformaLogConsola(), configPlataformaLogNivel() );
	//log_info(LOGGER, "INICIALIZANDO Plataforma '%s' ", configPlataformaNombre());


	return EXIT_SUCCESS;
}
