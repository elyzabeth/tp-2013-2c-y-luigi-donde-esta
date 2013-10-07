/*
 * configNivel.c
 *
 *  Created on: Sep 19, 2013
 *      Author: elizabeth
 */

#include "configNivel.h"


typedef struct configNivel {
	int32_t TIEMPOCHEQUEODEADLOCK;
	int32_t ENEMIGOS;
	int32_t SLEEP_ENEMIGOS;
	int32_t QUANTUM;
	int32_t RETARDO;
	int32_t PLATAFORMAPUERTO;
	char RECOVERY;
	char NOMBRE[MAXCHARLEN+1]; 		//Nombre del nivel ej: nombre=Nivel1
	char PLATAFORMA[MAXCHARLEN+1];
	char PLATAFORMAIP[MAXCHARLEN+1];
	char ALGORITMO[MAXCHARLEN+1];
	char LOG_PATH[MAXCHARLEN+1]; 	// LOG_PATH=/tmp/plataforma.log
	t_log_level LOG_NIVEL; 	// LOG_NIVEL=TRACE|DEBUG|INFO|WARNING|ERROR
	int32_t LOG_CONSOLA;			// LOG_CONSOLA=0|1 (off/on)
	t_dictionary *RECURSOS;

} t_configNivel;

t_configNivel configNivel;

void inicializarConfigNivel () {
	configNivel.TIEMPOCHEQUEODEADLOCK = 0;
	configNivel.ENEMIGOS = 0;
	configNivel.SLEEP_ENEMIGOS = 0;
	configNivel.QUANTUM = 0;
	configNivel.RETARDO = 0;
	configNivel.PLATAFORMAPUERTO = 0;
	configNivel.RECOVERY = 0;
	configNivel.NOMBRE[0]='\0';
	configNivel.PLATAFORMA[0]='\0';
	configNivel.PLATAFORMAIP[0]='\0';
	configNivel.ALGORITMO[0]='\0';
	configNivel.LOG_PATH[0]='\0';
	configNivel.LOG_NIVEL=0;
	configNivel.LOG_CONSOLA = 0;
	configNivel.RECURSOS = dictionary_create();
}



// GETTERS
// ********

/**
 * @NAME: configNivelNombre
 * @DESC: Devuelve Valor del campo Nombre del archivo de configuracion
 * Representa el Nombre del Nivel
 * ej:Nombre=nivel1
 */
char* configNivelNombre() {
	return configNivel.NOMBRE;
}

/**
 * @NAME: configNivelIpPuertoPlataforma
 * @DESC: Devuelve Valor del campo Plataforma del archivo de configuracion
 * Representa Ip y Puerto de la plataforma (orquestador) en el formato IP:PUERTO
 * ej:192.168.0.12:5000
 */
char* configNivelPlataforma() {
	return configNivel.PLATAFORMA;
}

/**
 * @NAME: configNivelIpPlataforma
 * @DESC: Devuelve Valor del campo PLATAFORMAIP
 * Representa Ip de la plataforma (orquestador)
 *  en el formato XXX.XXX.XXX.XXX ej:192.168.0.12
 */
char* configNivelPlataformaIp() {
	return configNivel.PLATAFORMAIP;
}

/**
 * @NAME: configNivelPuertoPlataforma
 * @DESC: Devuelve Valor del campo PLATAFORMAPUERTO
 * Representa Puerto de la plataforma (orquestador)
 * ej: 5000
 */
int32_t configNivelPlataformaPuerto() {
	return configNivel.PLATAFORMAPUERTO;
}

/**
 * @NAME: configNivelTiempoChequeoDeadlock
 * @DESC: Devuelve Valor del campo TiempoChequeoDeadlock del archivo de configuracion
 * Representa el Tiempo de chequeo de interbloqueo (en ms)
 * ej: TiempoChequeoDeadlock=10000
 */
int32_t configNivelTiempoChequeoDeadlock() {
	return configNivel.TIEMPOCHEQUEODEADLOCK;
}

/**
 * @NAME: configNivelRecovery
 * @DESC: Devuelve Valor del campo Recovery del archivo de configuracion
 * Recovery (on/off): indica si estará activa la elección de víctima y su consecuente finalización.
 * ej: Recovery=1
 */
int32_t configNivelRecovery() {
	return configNivel.RECOVERY;
}

/**
 * @NAME: configNivelEnemigos
 * @DESC: Devuelve Valor del campo Enemigos del archivo de configuracion
 * Representa la cantidad de hilos enemigos
 * ej: Enemigos=3
 */
int32_t configNivelEnemigos() {
	return configNivel.ENEMIGOS;
}

/**
 * @NAME: configNivelSleepEnemigos
 * @DESC: Devuelve Valor del campo Sleep_Enemigos del archivo de configuracion
 * Representa Tiempo de movimientos de enemigos
 * ej: Sleep_Enemigos=2000
 */
int32_t configNivelSleepEnemigos() {
	return configNivel.SLEEP_ENEMIGOS;
}

/**
 * @NAME: configNivelAlgoritmo
 * @DESC: Devuelve Valor del campo Algoritmo del archivo de configuracion
 * Representa el algoritmo de Planificacion de los procesos Personajes y enemigos.??
 * ej: algoritmo=RR|SRDF (Round Robin o Shortest Remaining Distance First)
 */
char* configNivelAlgoritmo() {
	return configNivel.ALGORITMO;
}

/**
 * @NAME: configNivelQuantum
 * @DESC: Devuelve Valor del campo quantum del archivo de configuracion
 * Representa el valor del quantum para el algoritmo de Planificacion Round Robin
 * ej: quantum=3
 */
int32_t configNivelQuantum() {
	return configNivel.QUANTUM;
}

/**
 * @NAME: configNivelRetardo
 * @DESC: Devuelve Valor del campo retardo del archivo de configuracion
 * Representa el tiempo de retardo entre turnos del personaje
 * (Valor en milisegundos que cada planificador de manera independiente deberá esperar entre cada asignación de turno)
 * ej: retardo=500
 */
int32_t configNivelRetardo() {
	return configNivel.RETARDO;
}

/**
 * @NAME: configNivelLogPath
 * @DESC: Devuelve Valor del campo LOG_PATH del archivo de configuracion
 * Representa el path del archivo de log para el Nivel.
 * ej: PATH_LOG=/tmp/nivel1.log
 */
char* configNivelLogPath() {
	return configNivel.LOG_PATH;
}

/**
 * @NAME: configNivelLogNivel
 * @DESC: Devuelve Valor del campo LOG_NIVEL del archivo de configuracion
 * Representa el nivel de logueo del archivo de log para el Nivel.
 * ej: NIVEL_LOG=DEBUG (DEBUG|WARN|ERROR)
 */
t_log_level configNivelLogNivel() {
	return configNivel.LOG_NIVEL;
}

/**
 * @NAME: configNivelLogPantalla
 * @DESC: Devuelve Valor del campo LOG_CONSOLA del archivo de configuracion
 * Representa el valor de si el log tambien se ve por pantalla true/false
 * ej: LOG_CONSOLA=1|0 (true/false)
 */
int32_t configNivelLogConsola() {
	return configNivel.LOG_CONSOLA;
}

/**
 * @NAME: configNivelRecurso
 * @DESC: Devuelve el recurso (t_caja) asociado al simbolo
 */
t_caja* configNivelRecurso(char simboloRecurso) {
	// TODO Modificar para que devuelva una copia.
	char simbolo[2] = {0};
	simbolo[0] = simboloRecurso;
	return (t_caja*)dictionary_get(configNivel.RECURSOS, simbolo);
}

/**
 * @NAME: configNivelRecursos
 * @DESC: Devuelve el listado de recursos del Nivel (t_dictionary)
 */
t_dictionary* configNivelRecursos() {
	// TODO Modificar para que devuelva una copia.
	return configNivel.RECURSOS;
}




/// FUNCIONES PRIVADAS
// *********************

t_caja* crearCaja() {
	return (t_caja*)malloc(sizeof(t_caja));
}

void destruirCaja(t_caja* caja) {
	free(caja);
}

void destruirConfigNivel () {
//	dictionary_clean_and_destroy_elements(configNivel.RECURSOS, (void*)destruirCaja);
//	dictionary_destroy(configNivel.RECURSOS);
	dictionary_destroy_and_destroy_elements(configNivel.RECURSOS, free);
}

void agregarRecurso (t_caja* caja) {
	// Utilizo el simbolo del recurso como key para el diccionario
	char simbolo[2] = {0};
	simbolo[0] = caja->SIMBOLO;
	dictionary_put(configNivel.RECURSOS, simbolo, caja);
}

void GenerarListaRecursos(t_config *config) {
	char key[10] = { 0 };
	int i;
	char atributos[MAXCHARLEN+1];
	char** substring;

	for(i = 0; i < MAXCANTCAJAS; i++) {
		sprintf(key, "%s%d", "Caja", i);
		if (config_has_property(config, key)) {
			t_caja* caja = crearCaja();
			strcpy(atributos, config_get_string_value(config, key));
			substring = string_split(atributos, ",");

			// # CajaNNN=[Nombre],[Simbolo],[Instancias],[PosX],[PosY]
			// ej: Caja1=Flores,F,3,23,5
			strcpy(caja->NOMBRE, key);
			strcpy(caja->RECURSO, substring[0]);
			caja->SIMBOLO= substring[1][0];
			caja->INSTANCIAS= atoi(substring[2]);
			caja->POSX= atoi(substring[3]);
			caja->POSY= atoi(substring[4]);

			agregarRecurso(caja);

			string_iterate_lines(substring, (void*) free);
			free(substring);
		}
	}
}



void levantarArchivoConfiguracionNivel () {
	t_config *config;
	config = config_create(PATH_CONFIG_NIVEL);

	if (config->properties->elements_amount == 0) {
		printf("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION %s\n", PATH_CONFIG_NIVEL);
		perror("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION\n( Don't PANIC! Te olvidaste de ejecutar:  ln -s ../nivel.conf nivel.conf )\n\n");
		config_destroy(config);
		exit(-1);
	}

	//Inicializo Estructura
	inicializarConfigNivel(configNivel);

	//Levanto los parametros necesarios para el Nivel
	strcpy(configNivel.NOMBRE, config_get_string_value(config, "Nombre"));

	strcpy(configNivel.PLATAFORMA, config_get_string_value(config, "Plataforma"));
	//SPLIT DE PLATAFORMA PARA SEPARAR IP DE PUERTO
	separarIpPuerto(configNivel.PLATAFORMA, configNivel.PLATAFORMAIP, &(configNivel.PLATAFORMAPUERTO));

	configNivel.TIEMPOCHEQUEODEADLOCK = config_get_int_value(config, "TiempoChequeoDeadlock");
	configNivel.RECOVERY = config_get_int_value(config, "Recovery");
	configNivel.ENEMIGOS = config_get_int_value(config, "Enemigos");
	configNivel.SLEEP_ENEMIGOS = config_get_int_value(config, "Sleep_Enemigos");

	strcpy(configNivel.ALGORITMO, config_get_string_value(config, "algoritmo"));
	configNivel.QUANTUM = config_get_int_value(config, "quantum");
	configNivel.RETARDO = config_get_int_value(config, "retardo");

	strcpy(configNivel.LOG_PATH, config_get_string_value(config, "LOG_PATH"));
	configNivel.LOG_NIVEL = obtenerLogLevel(config_get_string_value(config, "LOG_NIVEL"));
	configNivel.LOG_CONSOLA = config_get_int_value(config, "LOG_CONSOLA");

	// Armo lista dinamica de recursos
	GenerarListaRecursos(config);

	// Una vez que se levantaron los datos del archivo de configuracion
	// puedo/debo destruir la estructura config.
	config_destroy(config);
}

void levantarCambiosArchivoConfiguracionNivel () {
	t_config *config;
	config = config_create(PATH_CONFIG_NIVEL);

	if (config->properties->elements_amount == 0) {
		printf("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION %s ", PATH_CONFIG_NIVEL);
		perror("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION\n\n");
		config_destroy(config);
		exit(-1);
	}

	strcpy(configNivel.ALGORITMO, config_get_string_value(config, "algoritmo"));
	configNivel.QUANTUM = config_get_int_value(config, "quantum");
	configNivel.RETARDO = config_get_int_value(config, "retardo");

	configNivel.LOG_NIVEL = obtenerLogLevel(config_get_string_value(config, "LOG_NIVEL"));
	configNivel.LOG_CONSOLA = config_get_int_value(config, "LOG_CONSOLA");

	// Una vez que se levantaron los datos del archivo de configuracion
	// puedo/debo destruir la estructura config.
	config_destroy(config);
}

