/*
 * personaje.c
 *
 *  Created on: Sep 24, 2013
 *      Author: elizabeth
 */

#include "personaje.h"



int main (int argc, char *argv[]) {

	inicializarPersonaje();

	// TODO agregar logica del personaje
	personaje_t personaje;
	int id_proceso;
	int sock = -1;
	char *buffer_header;
	header_t header;
	/************************************ Inicio **************************************/
	/*
	  if(argc != 3)
	  {
	    	puts("Error cantidad Parametros");
	  }
	*/
		id_proceso = getpid();
		system("clear");
		printf("************** Iniciando Personaje (PID: %d) ***************\n",id_proceso);
		cambiar_nombre_proceso(argv,argc, personaje.nombre);

	/***************** ME CONECTO Y ARMO MENSAJE DE PRESENTACION *******/
		printf("************** CONECTANDOCE  ***************\n");

		//conectar(personaje.ip, personaje.puerto, &sock);
		conectar("127.0.0.1", 1500, &sock);

		header.tipo = NUEVO_PERSONAJE;
		header.largo_mensaje = 0/*TAMPRESENT*/;

	   	buffer_header = malloc(sizeof(header_t)/*TAMHEADER*/); /*primera y unica vez */
		memset(buffer_header, '\0', sizeof(header_t)/*TAMHEADER*/);
		memcpy(buffer_header, &header, sizeof(header_t)/*TAMHEADER*/);

		printf("sizeof(header): %d, largo mensaje: %d  buffer_header:%d\n", sizeof(header), header.largo_mensaje, sizeof(&buffer_header));


		if (enviar(sock, buffer_header, sizeof(header_t)) != EXITO)
		{
			printf("Error al enviar header NUEVO_PERSONAJE\n\n");
			return WARNING;
		}

		while(1)
		{

			memset(buffer_header, '\0', sizeof(header_t));
			//memset(header, '\0', sizeof(header_t)); // se puede?

			recibir (sock, buffer_header, sizeof(header_t));
			memcpy(&header, buffer_header, sizeof(header_t));

			//free (buffer_header); Como siempre lo uso libero al final

			switch (header.tipo) /*recibo estado */
			{
				case PERSONAJE_CONECTADO: puts ("Personaje Conectado");
								break;

				case OTRO: 	puts ("otro");
								break;


			}
		}

		free (buffer_header); /* Solo al final porque lo uso siempre */
		//RETURN 0;


	finalizarPersonaje();

	return EXIT_SUCCESS;
}

/**
 * @NAME: inicializarPersonaje
 * @DESC: Inicializa todas las variables y estructuras necesarias para el proceso personaje
 */
void inicializarPersonaje() {
	levantarArchivoConfiguracionPersonaje();
	// TODO agregar inicializaciones necesarias
	LOGGER = log_create(configPersonajeLogPath(), "PERSONAJE", configPersonajeLogConsola(), configPersonajeLogNivel() );
	log_info(LOGGER, "INICIALIZANDO PERSONAJE '%s' ", configPersonajeNombre());

}

/**
 * @NAME: finalizarPersonaje
 * @DESC: Finaliza todas las variables y estructuras que fueron creadas para el proceso personaje
 */
void finalizarPersonaje() {
	log_info(LOGGER, "FINALIZANDO PERSONAJE\n");
	destruirConfigPersonaje();
	log_destroy(LOGGER);
}
