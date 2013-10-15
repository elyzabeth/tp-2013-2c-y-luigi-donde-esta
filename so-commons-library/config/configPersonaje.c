/*
 * configPersonaje.c
 *
 *  Created on: 22/09/2013
 *      Author: arwen
 */

#include "configPersonaje.h"

// Estructura privada
typedef struct {
	char NOMBRE[MAXCHARLEN+1];			// nombre=Mario
	char SIMBOLO;						// simbolo=@
	int32_t VIDAS;						// vidas=5
	t_queue *PLANDENIVELES;				// planDeNiveles=[Nivel3,Nivel4,Nivel1]
	char PLATAFORMA[MAXCHARLEN+1];		// orquestador=192.168.0.100:5000
	char PLATAFORMAIP[MAXCHARLEN+1];	//
	int32_t PLATAFORMAPUERTO;			//
	char LOG_PATH[MAXCHARLEN+1];		//ej LOG_PATH=/tmp/plataforma.log
	t_log_level LOG_NIVEL;				//ej LOG_NIVEL=TRACE|DEBUG|INFO|WARNING|ERROR
	int32_t LOG_CONSOLA;				// LOG_CONSOLA=0|1 (off/on)
} t_configPersonaje;

t_configPersonaje configPersonaje;


// Declaracion de prototipos privados
t_queue* clonarColaPlan(t_queue* planDeNiveles);


// FUNCIONES
void inicializarconfigPersonaje () {

	configPersonaje.NOMBRE[0]='\0';
	configPersonaje.SIMBOLO=0;
	configPersonaje.VIDAS = 0;
	configPersonaje.PLANDENIVELES = queue_create();
	configPersonaje.PLATAFORMA[0]='\0';
	configPersonaje.PLATAFORMAIP[0]='\0';
	configPersonaje.PLATAFORMAPUERTO = 0;
	configPersonaje.LOG_PATH[0]='\0';
	configPersonaje.LOG_NIVEL=0;
	configPersonaje.LOG_CONSOLA=0;

}


// GETTERS
// **************

/**
 * @NAME configPersonajeNombre
 */
const char * configPersonajeNombre() {
	return configPersonaje.NOMBRE;
}

char configPersonajeSimbolo() {
	return configPersonaje.SIMBOLO;
}

int32_t configPersonajeVidas() {
	return configPersonaje.VIDAS;
}

t_queue* configPersonajePlanDeNiveles() {
	return clonarColaPlan(configPersonaje.PLANDENIVELES);
}

const char * configPersonajePlataforma() {
	return configPersonaje.PLATAFORMA;
}

const char * configPersonajePlataformaIp() {
	return configPersonaje.PLATAFORMAIP;
}

int32_t configPersonajePlataformaPuerto() {
	return configPersonaje.PLATAFORMAPUERTO;
}

char * configPersonajeLogPath() {
	return configPersonaje.LOG_PATH;
}

int32_t configPersonajeLogNivel() {
	return configPersonaje.LOG_NIVEL;
}

int32_t configPersonajeLogConsola() {
	return configPersonaje.LOG_CONSOLA;
}



// *********************************
/// FUNCIONES PRIVADAS
// *********************************

void destruirConfigPersonaje () {
	queue_destroy_and_destroy_elements(configPersonaje.PLANDENIVELES, (void*)free);
}

t_queue* clonarColaPlan(t_queue* planDeNiveles) {
	int i;
	t_queue* clon = queue_create();
	t_objetivosxNivel* oxn;

	for (i = 0 ; i < queue_size(planDeNiveles); i++) {

		t_objetivosxNivel* copia = (t_objetivosxNivel*)calloc(1,sizeof(t_objetivosxNivel));
		oxn = (t_objetivosxNivel*)queue_pop(planDeNiveles);
		queue_push(planDeNiveles, oxn);

		strcpy(copia->nivel, oxn->nivel);
		strcpy(copia->objetivos, oxn->objetivos);
		copia->totalObjetivos = oxn->totalObjetivos;

		queue_push(clon, copia);
	}

	return clon;
}

void GenerarPlanDeNiveles(t_config *config) {
	// Armo la lista FIFO dinamica del Plan de niveles y los objetivos.
	//char plan[MAXCHARLEN+1]={0};
	//char objetivos[200+1]={0};
	char key[20] = { 0 };
	char** substring;
	char** recursos;

	// planDeNiveles=[Nivel3,Nivel4,Nivel1]
	// obj[Nivel1]=[F,H,F,M]
	// obj[Nivel3]=[C,J,C]
	// obj[Nivel4]=[P,Q,M]

	// Quito los corchetes de la expresion "[Nivel3,Nivel4,Nivel1]"
	//quitarCorchetes(plan, config_get_string_value(config, "planDeNiveles"));
	//substring = string_split(plan, ",");
	substring = string_get_string_as_array(config_get_string_value(config, "planDeNiveles"));

	void _add_objetives(char *nivel) {
		int32_t cantObjetivos = 0;
		//int i=0;
		t_objetivosxNivel *objxniv;
		objxniv = (t_objetivosxNivel*)calloc(1, sizeof(t_objetivosxNivel));
		strcpy(objxniv->nivel, nivel);
		sprintf(key, "obj[%s]", nivel );

		// Quito los corchetes de la expresion "[F,H,F,M]"
		//quitarCorchetes(objetivos, config_get_string_value(config, key));
		//recursos = string_split(objetivos, ",");
		recursos = string_get_string_as_array(config_get_string_value(config, key));

		void _add_resource(char *rec) {
			objxniv->objetivos[cantObjetivos] = rec[0];
			cantObjetivos++;
		}

		string_iterate_lines(recursos, _add_resource);
		objxniv->totalObjetivos = cantObjetivos;

		// Agrego a la cola, el Nivel con sus objetivos
		queue_push(configPersonaje.PLANDENIVELES, objxniv);

		string_iterate_lines(recursos, (void*)free);
		free(recursos);

	}

	string_iterate_lines(substring, _add_objetives);
	string_iterate_lines(substring, (void*) free);
	free(substring);

}

void levantarArchivoConfiguracionPersonaje () {
	t_config *config;
	char simbolo[2];
	config = config_create(PATH_CONFIG_PERSONAJE);

	if (config->properties->elements_amount == 0) {
		printf("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION %s \n", PATH_CONFIG_PERSONAJE);
		perror("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION \n( Don't PANIC! Si estas por consola ejecutar: ln -s ../personaje.conf personaje.conf )\n\n");
		config_destroy(config);
		exit(-1);
	}

	//Inicializo Estructura
	inicializarconfigPersonaje(configPersonaje);

	//Levanto los parametros necesarios para el Personaje
	strcpy(configPersonaje.NOMBRE, config_get_string_value(config, "nombre"));
	strncpy(simbolo, config_get_string_value(config, "simbolo"), 1);
	configPersonaje.SIMBOLO = simbolo[0];

	strcpy(configPersonaje.PLATAFORMA, config_get_string_value(config, "orquestador"));
	//SPLIT DE PLATAFORMA PARA SEPARAR IP DE PUERTO
	separarIpPuerto(configPersonaje.PLATAFORMA, configPersonaje.PLATAFORMAIP, &(configPersonaje.PLATAFORMAPUERTO));

	configPersonaje.VIDAS = config_get_int_value(config, "vidas");

	strcpy(configPersonaje.LOG_PATH, config_get_string_value(config, "LOG_PATH"));
	configPersonaje.LOG_NIVEL = obtenerLogLevel(config_get_string_value(config, "LOG_NIVEL"));
	configPersonaje.LOG_CONSOLA = config_get_int_value(config, "LOG_CONSOLA");

	// Armo lista dinamica de recursos
	GenerarPlanDeNiveles(config);

	// Una vez que se levantaron los datos del archivo de configuracion
	// puedo/debo destruir la estructura config.
	config_destroy(config);
}
