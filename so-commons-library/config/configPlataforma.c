/*
 * configPlataforma.c
 *
 *  Created on: Sep 19, 2013
 *      Author: elizabeth
 */

#include "configPlataforma.h"

typedef struct {
	int32_t PUERTO;					// PUERTO=5000
	char KOOPA[MAXCHARLEN+1];		// KOOPA=/home/utnso/koopa
	char SCRIPT[MAXCHARLEN+1];		// SCRIPT=/home/utnso/evaluacion.sh
	char LOG_PATH[MAXCHARLEN+1];	// LOG_PATH=/tmp/plataforma.log
	t_log_level LOG_NIVEL;			// LOG_NIVEL=TRACE|DEBUG|INFO|WARNING|ERROR
	int32_t LOG_CONSOLA;			// LOG_CONSOLA=0|1 (off/on)
} t_configPlat;

t_configPlat configPlat;

void inicializarConfigPlat () {
	configPlat.PUERTO = 0;
	configPlat.KOOPA[0]='\0';
	configPlat.SCRIPT[0]='\0';
	configPlat.LOG_PATH[0]='\0';
	configPlat.LOG_NIVEL=0;
	configPlat.LOG_CONSOLA=0;
}

void destruirConfigPlataforma(){
	// No hay estructuras dinamicas a destruir...
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
const char* configPlatKoopa () {
	return configPlat.KOOPA;
}

/**
 * @NAME: configPlatScript
 * @DESC: Devuelve Valor del campo Script del archivo de configuracion
 * Representa el Path al archivo
 * ej: Script=/utnso/script.sh
 */
const char* configPlatScript (){
	return configPlat.SCRIPT;
}

/**
 * @NAME: configPlatLogPath
 * @DESC: Devuelve Valor del campo Path_log del archivo de configuracion
 * Representa el Path al archivo de log
 * ej: PATH_LOG=/utnso/plataforma.log
 */
const char* configPlatLogPath (){
	return configPlat.LOG_PATH;
}

/**
 * @NAME: configPlatLogNivel
 * @DESC: Devuelve Valor del campo Nivel_log del archivo de configuracion
 * Representa el nivel de logueo DEBUG|WARN|ERROR
 * ej: NIVEL_LOG=DEBUG
 */
t_log_level configPlatLogNivel (){
	return configPlat.LOG_NIVEL;
}

/**
 * @NAME: configPlatLogConsola
 * @DESC: Devuelve Valor del campo LOG_CONSOLA del archivo de configuracion
 * Representa el valor de si el log tambien se ve por pantalla true/false
 * ej: LOG_CONSOLA=1|0 (true/false)
 */
int32_t configPlatLogConsola (){
	return configPlat.LOG_CONSOLA;
}




void levantarArchivoConfiguracionPlataforma() {
	t_config *config;
	config = config_create(PATH_CONFIG_PLATAFORMA);

	if (config->properties->elements_amount == 0) {
		printf("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION %s \n", PATH_CONFIG_PLATAFORMA);
		perror("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION\n( Don't PANIC! Si estas por consola ejecuta: ln -s ../plataforma.conf plataforma.conf )\n\n");
		config_destroy(config);
		exit(-1);
	}

	//Levanto los parametros necesarios para el planificador
	configPlat.PUERTO = config_get_int_value(config, "PUERTO");

	strcpy(configPlat.KOOPA, config_get_string_value(config, "KOOPA"));
	strcpy(configPlat.SCRIPT, config_get_string_value(config, "SCRIPT"));

	strcpy(configPlat.LOG_PATH, config_get_string_value(config, "LOG_PATH"));
	configPlat.LOG_NIVEL = obtenerLogLevel( config_get_string_value(config, "LOG_NIVEL"));
	configPlat.LOG_CONSOLA = config_get_int_value(config, "LOG_CONSOLA");

	// Una vez que se levantaron los datos del archivo de configuracion
	// puedo/debo destruir la estructura config.
	config_destroy(config);
}



