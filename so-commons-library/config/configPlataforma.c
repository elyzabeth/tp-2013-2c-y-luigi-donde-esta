/*
 * configPlataforma.c
 *
 *  Created on: Sep 19, 2013
 *      Author: elizabeth
 */

#include "configPlataforma.h"

typedef struct {
	int32_t PUERTO;   //ej PUERTO=5000
	char KOOPA[MAXCHARLEN+1];  //ej KOOPA=/home/utnso/koopa
	char SCRIPT[MAXCHARLEN+1]; //ej SCRIPT=/home/utnso/evaluacion.sh
	char PATH_LOG[MAXCHARLEN+1]; //ej PATH_LOG=/tmp/plataforma.log
	char NIVEL_LOG[MAXCHARLEN+1]; //ej NIVEL_LOG=DEBUG|WARN|ERROR
} t_configPlat;

t_configPlat configPlat;

void inicializarConfigPlat () {
	configPlat.PUERTO = 0;
	configPlat.KOOPA[0]='\0';
	configPlat.SCRIPT[0]='\0';
	configPlat.PATH_LOG[0]='\0';
	configPlat.NIVEL_LOG[0]='\0';
}

/**
 * @NAME: configPlatPuerto
 * @DESC: Devuelve Valor del campo Puerto del archivo de configuracion
 * Representa el Puerto de escucha de la Plataforma
 * ej:Puerto=5000
 */
int32_t configPlatPuerto (){
	return configPlat.PUERTO;
}

/**
 * @NAME: configPlatKoopa
 * @DESC: Devuelve Valor del campo Koopa del archivo de configuracion
 * Representa el Path al archivo koopa
 * ej:Koopa=/utnso/koopa
 */
char* configPlatKoopa () {
	return configPlat.KOOPA;
}

/**
 * @NAME: configPlatScript
 * @DESC: Devuelve Valor del campo Script del archivo de configuracion
 * Representa el Path al archivo
 * ej: Script=/utnso/script.sh
 */
char* configPlatScript (){
	return configPlat.SCRIPT;
}

/**
 * @NAME: configPlatPathLog
 * @DESC: Devuelve Valor del campo Path_log del archivo de configuracion
 * Representa el Path al archivo de log
 * ej: PATH_LOG=/utnso/plataforma.log
 */
char* configPlatPathLog (){
	return configPlat.PATH_LOG;
}

/**
 * @NAME: configPlatNivelLog
 * @DESC: Devuelve Valor del campo Nivel_log del archivo de configuracion
 * Representa el nivel de logueo DEBUG|WARN|ERROR
 * ej: NIVEL_LOG=DEBUG
 */
char* configPlatNivelLog (){
	return configPlat.NIVEL_LOG;
}


void levantarArchivoConfiguracionPlataforma() {
	t_config *config;
	config = config_create(PATH_CONFIG_PLATAFORMA);

	if (config->properties->elements_amount == 0) {
		printf("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION %s ", PATH_CONFIG_PLATAFORMA);
		perror("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION");
		config_destroy(config);
		exit(-1);
	}

	//Levanto los parametros necesarios para el planificador
	configPlat.PUERTO = config_get_int_value(config, "PUERTO");

	strcpy(configPlat.KOOPA, config_get_string_value(config, "KOOPA"));
	strcpy(configPlat.SCRIPT, config_get_string_value(config, "SCRIPT"));
	strcpy(configPlat.PATH_LOG, config_get_string_value(config, "PATH_LOG"));
	strcpy(configPlat.NIVEL_LOG, config_get_string_value(config, "NIVEL_LOG"));

	// Una vez que se levantaron los datos del archivo de configuracion
	// puedo/debo destruir la estructura config.
	config_destroy(config);
}



