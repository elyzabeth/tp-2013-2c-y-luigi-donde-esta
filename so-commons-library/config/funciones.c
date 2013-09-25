/*
 * funciones.c
 *
 *  Created on: 22/09/2013
 *      Author: elyzabeth
 */

#include "funciones.h"

/**
 * @NAME: separarIpPuerto
 * @DESC: Separa el valor Ip del valor Puerto
 * RECIBE <IP>:<PUERTO>
 * ej: 192.168.0.10:5000
 */
void separarIpPuerto(char *ipPuerto, char *ip, int32_t *puerto) {
	char** substring;

	substring = (char**)string_split(ipPuerto, ":");
	//strcpy(configNivel.PLATAFORMAIP, substring[0]);
	strcpy(ip, substring[0]);
	//configNivel.PLATAFORMAPUERTO = atoi(substring[1]);
	*puerto = atoi(substring[1]);

	string_iterate_lines(substring, (void*) free);
	free(substring);
}

t_log_level obtenerLogLevel (char *LOG_NIVEL) {
	// TRACE|DEBUG|INFO|WARNING|ERROR
	if(strcmp(LOG_NIVEL, "TRACE") == 0) return LOG_LEVEL_TRACE;
	if(strcmp(LOG_NIVEL, "DEBUG") == 0) return LOG_LEVEL_DEBUG;
	if(strcmp(LOG_NIVEL, "INFO") == 0) return LOG_LEVEL_INFO;
	if(strcmp(LOG_NIVEL, "WARNING") == 0) return LOG_LEVEL_WARNING;
	if(strcmp(LOG_NIVEL, "ERROR") == 0) return LOG_LEVEL_ERROR;
	// valor por defecto
	return LOG_LEVEL_TRACE;
}

void quitarCorchetes (char *to, char *from) {
	int i = 0, j = 0;

	while ( i < strlen(from) ){
		if(from[i] != ']' && from[i] != '[')
			to[j++] = from[i++];
		else
			i++;
	}

	to[j] = '\0';

}
