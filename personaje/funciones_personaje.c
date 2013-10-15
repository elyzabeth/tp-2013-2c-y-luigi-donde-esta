/*
 * funciones_personaje.c
 *
 *  Created on: 14/10/2013
 *      Author: elyzabeth
 */

#include "personaje.h"

int enviarMsjNuevoPersonaje( int sock ) {
	header_t header;

	memset(&header, '\0', sizeof(header_t));
	header.tipo = NUEVO_PERSONAJE;
	header.largo_mensaje = 0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	log_debug(LOGGER,"sizeof(header): %d, largo mensaje: %d  buffer_header: %lu\n", sizeof(header), header.largo_mensaje, sizeof(&buffer_header));

	return enviar(sock, buffer_header, sizeof(header_t));
}

int enviarInfoPersonaje(sock) {
	// PRUEBA
	header_t header;
	t_personaje yo;
	char* buffer;
	log_debug(LOGGER, "Envio mensaje de prueba con info del personaje");

	log_debug(LOGGER, "Datos: (%s, %d, %c)",  configPersonajeNombre(),configPersonajeSimbolo(), configPersonajeSimbolo());
	buffer = calloc(1, sizeof(header_t));
	strcpy(yo.nombre, configPersonajeNombre());
	yo.id = configPersonajeSimbolo();
	yo.posActual.x = 0;
	yo.posActual.y = 0;
	yo.fd = 0;
	strcpy( yo.nivel, "Nivel1");

	memset(&header, '\0', sizeof(header_t));
	header.tipo = CONECTAR_NIVEL;
	header.largo_mensaje = sizeof(t_personaje);

	memcpy(buffer, &header, sizeof(header_t));

	log_debug(LOGGER, "Envio header CONECTAR_NIVEL %d", sizeof(header_t));
	if (enviar(sock, buffer, sizeof(header_t)) != EXITO)
	{
		log_error(LOGGER,"Error al enviar header CONECTAR_NIVEL\n\n");
		free(buffer);
		return WARNING;
	}

	free(buffer);
	buffer = calloc(1, sizeof(t_personaje));
	memcpy(buffer, &yo, sizeof(t_personaje));

	log_debug(LOGGER, "Envio t_personaje %d (%s, %c, %d, %d, %d, %s)", sizeof(t_personaje), yo.nombre, yo.id, yo.posActual.x, yo.posActual.y, yo.fd, yo.nivel);
	if (enviar(sock, buffer, sizeof(t_personaje)) != EXITO)
	{
		log_error(LOGGER,"Error al enviar informacion del personaje\n\n");
		free(buffer);
		return WARNING;
	}

	free(buffer);

	return EXITO;
}
