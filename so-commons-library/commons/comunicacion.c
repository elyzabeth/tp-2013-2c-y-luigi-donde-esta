#include "comunicacion.h"

header_t* crearHeader() {
	header_t *nuevoHeader;
	nuevoHeader = calloc(1, sizeof(header_t));
	return nuevoHeader;
}

void initHeader(header_t* header) {
	memset(header, '\0', sizeof(header_t));
}

int enviar(int sock, char *buffer, int tamano)
{
	int escritos;

	if ((escritos = send (sock, buffer, tamano, 0)) <= 0)
	{
		printf("Error en el send\n\n");
		return WARNING;
	}
	else if (escritos != tamano)
	{
		printf("La cantidad de bytes enviados es distinta de la que se quiere enviar\n\n");
		return WARNING;
	}

	return EXITO;
}


int recibir(int sock, char *buffer, int tamano)
{
	int val;
	int leidos = 0;

	memset(buffer, '\0', tamano);

	while (leidos < tamano)
	{

		val = recv(sock, buffer + leidos, tamano - leidos, 0);
		leidos += val;
		if (val < 0)
		{
			printf("Error al recibir datos: %d - %s\n", val, strerror(val)); //ENOTCONN ENOTSOCK
			switch(val) {
				// The  socket  is  marked non-blocking and the receive operation would block, or a receive timeout had been set and the timeout expired before data was received.  POSIX.1-2001 allows either error to be returned for this
				// case, and does not require these constants to have the same value, so a portable application should check for both possibilities.
				//case EAGAIN: printf(" - EAGAIN \n The  socket  is  marked non-blocking and the receive operation would block, or a receive timeout had been set and the timeout expired before data was received.\n\n"); break;
				//case EWOULDBLOCK: printf("EWOULDBLOCK \n The  socket  is  marked non-blocking and the receive operation would block, or a receive timeout had been set and the timeout expired before data was received.\n\n"); break;
				// The argument sockfd is an invalid descriptor.
				case EBADF: printf("EBADF \n The argument sockfd is an invalid descriptor.\n\n"); break;
				// A remote host refused to allow the network connection (typically because it is not running the requested service).
				case ECONNREFUSED: printf("ECONNREFUSED \n A remote host refused to allow the network connection (typically because it is not running the requested service).\n\n"); break;
				// The receive buffer pointer(s) point outside the process's address space.
				case EFAULT: printf("EFAULT \n The receive buffer pointer(s) point outside the process's address space.\n\n"); break;
				// The receive was interrupted by delivery of a signal before any data were available; see signal(7).
				case EINTR: printf("EINTR \n The receive was interrupted by delivery of a signal before any data were available; see signal(7).\n\n"); break;
				// Invalid argument passed.
				case EINVAL: printf("EINVAL \n Invalid argument passed.\n\n"); break;
				// Could not allocate memory for recvmsg().
				case ENOMEM: printf("ENOMEM \n Could not allocate memory for recvmsg().\n\n"); break;
				// The socket is associated with a connection-oriented protocol and has not been connected (see connect(2) and accept(2)).
				case ENOTCONN: printf("ENOTCONN \n The socket is associated with a connection-oriented protocol and has not been connected (see connect(2) and accept(2)).\n\n"); break;
				// The argument sockfd does not refer to a socket.
				case ENOTSOCK: printf("ENOTSOCK \n The argument sockfd does not refer to a socket.\n\n"); break;

			}
			return ERROR;
		}
		// Cuando recv devuelve 0 es porque se desconecto el socket.
		if(val == 0)
		{
			/*printf("%d se desconecto\n", sock);*/
			return WARNING;
		}
	}
	return EXITO;
}

int conectar(char ip[15+1], int puerto, int *sock)
{
	struct sockaddr_in dirCent;
	//char *bufferMsg=NULL;
	//char *bufferMsg2=NULL;

	if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Error al crear el socket\n\n");
		return ERROR;
	}

	dirCent.sin_family = AF_INET;
	dirCent.sin_port = htons(puerto);
	dirCent.sin_addr.s_addr = inet_addr(ip);
	memset(&(dirCent.sin_zero), '\0', 8);

	if (connect(*sock, (struct sockaddr *)&dirCent, sizeof(struct sockaddr)) == -1)
	{
		printf("Imposible conectar\n\n");
		return ERROR;
	}
	return EXITO;

}

int aceptar_conexion(int *listener, int *nuevo_sock)
{
	struct sockaddr_in dirRemota;
	//size_t dirLong;
	socklen_t dirLong;
	dirLong = sizeof(dirRemota);

	if ((*nuevo_sock = accept(*listener, (struct sockaddr *)&dirRemota, &dirLong)) == -1)
	{
		puts("error accept");
		return ERROR;
	}

	return EXITO;
}

int enviar_header (int sock, header_t *header) {
	int ret;
	char *buffer = calloc(1,sizeof(header_t));
	memcpy(buffer, header, sizeof(header_t));

	if ((ret = enviar(sock, buffer, sizeof(header_t))) != EXITO)
	{
		printf("enviar_header: ERROR al enviar header_t al fd %d\n\n", sock);
		free(buffer);
		return WARNING;
	}

	free(buffer);

	return ret;
}

int recibir_header(int sock, header_t *header, fd_set *master/*por si se desconecta*/, int *seDesconecto)
{
	int ret;
	char *buffer = NULL;
	//char strAux[50];

	buffer = calloc(1, sizeof(header_t));
	*seDesconecto = FALSE; /*False =0 define*/

	ret = recibir(sock, buffer, sizeof(header_t));

	// WARNING es cuando se desconecto el socket.
	if (ret == WARNING) {
		//	sprintf(strAux, "Se desconecto el socket: %d\n", sock);
		FD_CLR(sock, master);
		close(sock);
		*seDesconecto = TRUE;
		free(buffer);
		return EXITO;
	}

	if (ret == ERROR) {
		free(buffer);
		//return trazarError(errorTrace, "Error al recibir datos :S", ERROR,"comunicacion.h", "recibirHeader()");
		return ERROR;
	}

	memset(header, '\0', sizeof(header_t));
	memcpy(header, buffer, sizeof(header_t)); /*ojo que el memcopy si lo haces afuera el primer parametro tiene que tener &*/
	/* Por ejemplo si la estructua no fuera por referencia y fuera local, debes hacer asi:
	memcpy(&header, buffer, sizeof(header_t));*/

	//	printf("sock: %d --- largo: %d ---- ", sock, header->largo_mensaje);
	//	printf("tipo: %d\n", header->tipo);
	free(buffer);

	return EXITO;

}

int recibir_header_simple(int sock, header_t *header)
{
	int ret;
	char *buffer = NULL;

	buffer = calloc(1, sizeof(header_t));

	ret = recibir(sock, buffer, sizeof(header_t));

	if (ret != EXITO)
		return ret;

	memset(header, '\0', sizeof(header_t));
	memcpy(header, buffer, sizeof(header_t));

	free(buffer);

	return EXITO;

}

int enviar_nivel(int sock, t_nivel *nivel) {
	int ret;
	char *buffer_payload = calloc(1,sizeof(t_nivel));
	memcpy(buffer_payload, nivel, sizeof(t_nivel));

	if ((ret = enviar(sock, buffer_payload, sizeof(t_nivel))) != EXITO)
	{
		printf("enviar_nivel: ERROR al enviar t_nivel al fd %d\n\n", sock);
		free(buffer_payload);
		return WARNING;
	}

	free(buffer_payload);

	return ret;
}

int recibir_nivel(int sock, t_nivel *nivel, fd_set *master, int *seDesconecto)
{
	int ret;
	char *buffer = NULL;

	buffer = calloc(1, sizeof(t_nivel));
	*seDesconecto = FALSE; /*False =0 define*/

	//printf("Espero recibir t_nivel (%u)", sizeof(t_nivel));
	ret = recibir(sock, buffer, sizeof(t_nivel));

	if (ret == WARNING) {
		FD_CLR(sock, master);
		close(sock);
		*seDesconecto = TRUE;
		free(buffer);
		return WARNING;
	}

	if (ret == ERROR) {
		free(buffer);
		return ERROR;
	}

	memcpy(nivel, buffer, sizeof(t_nivel));

	free(buffer);

	return EXITO;

}

int enviar_personaje(int sock, t_personaje *personaje) {
	int ret;
	char *buffer_payload = calloc(1,sizeof(t_personaje));
	memcpy(buffer_payload, personaje, sizeof(t_personaje));

	if ((ret = enviar(sock, buffer_payload, sizeof(t_personaje))) != EXITO)
	{
		printf("enviar_nivel: ERROR al enviar t_personaje al fd %d\n\n", sock);
		free(buffer_payload);
		return WARNING;
	}

	free(buffer_payload);

	return ret;
}

int recibir_personaje(int sock, t_personaje *personaje, fd_set *master, int *seDesconecto)
{
	int ret;
	char *buffer = NULL;

	buffer = calloc(1, sizeof(t_personaje));
	*seDesconecto = FALSE; /*False =0 define*/

	//printf("Espero recibir t_personaje (%u)", sizeof(t_personaje));
	ret = recibir(sock, buffer, sizeof(t_personaje));

	if (ret == WARNING) {
		FD_CLR(sock, master);
		close(sock);
		*seDesconecto = TRUE;
		free(buffer);
		return WARNING;
	}

	if (ret == ERROR) {
		free(buffer);
		return ERROR;
	}

	memcpy(personaje, buffer, sizeof(t_personaje));

	free(buffer);

	return EXITO;

}

int enviar_caja(int sock, t_caja *caja) {
	int ret;
	char *buffer_payload = calloc(1,sizeof(t_caja));
	memcpy(buffer_payload, caja, sizeof(t_caja));

	if ((ret = enviar(sock, buffer_payload, sizeof(t_caja))) != EXITO)
	{
		printf("enviar_nivel: ERROR al enviar t_caja al fd %d\n\n", sock);
		free(buffer_payload);
		return WARNING;
	}

	free(buffer_payload);

	return ret;
}


int recibir_caja(int sock, t_caja *caja, fd_set *master, int *seDesconecto)
{
	int ret;
	char *buffer = NULL;

	buffer = calloc(1, sizeof(t_caja));
	*seDesconecto = FALSE; /*False =0 define*/

	//printf("Espero recibir t_caja (%u)", sizeof(t_caja));
	ret = recibir(sock, buffer, sizeof(t_caja));

	if (ret == WARNING) {
		FD_CLR(sock, master);
		close(sock);
		*seDesconecto = TRUE;
		free(buffer);
		return WARNING;
	}

	if (ret == ERROR) {
		free(buffer);
		return ERROR;
	}

	memcpy(caja, buffer, sizeof(t_caja));

	free(buffer);

	return EXITO;

}

int quitar_descriptor(int desc, fd_set *listaDesc, int *maxDesc)
{
	FD_CLR(desc, listaDesc);
	close(desc);

	//	if (desc == *maxDesc)
	//	{
	//		*maxDesc = *maxDesc-1;
	//	}
	return EXITO;
}

int agregar_descriptor(int desc, fd_set *listaDesc, int *maxDesc)
{

	FD_SET(desc, listaDesc);
	if (desc > *maxDesc)
	{
		*maxDesc = desc;
	}
	return EXITO;
}


int crear_listener(int puerto, int *listener)
{
	struct sockaddr_in miDir;
	int yes = 1;
	char reintentarConex = 1;

	if ((*listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Error al crear el Socket\n\n");
		return ERROR;
	}

	if (setsockopt(*listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		puts("Error en setsockopt()");
		return ERROR;
	}
	for(;reintentarConex;)
	{
		miDir.sin_family = AF_INET;
		miDir.sin_addr.s_addr = INADDR_ANY;
		miDir.sin_port = htons(puerto);
		memset(&(miDir.sin_zero), '\0', 8);

		if (bind(*listener, (struct sockaddr*)&miDir, sizeof(miDir)) == -1)
		{
			// *puerto++; tratamiento de error
			reintentarConex = 0;
		}
	}
	listen(*listener, 10);

	return EXITO;
}

void genId(char idMsg[])
{
	time_t tTiempo;
	//header_t msgId;

	int x;

	/*******Planto la semilla***********/
	tTiempo = time(&tTiempo) + (getpid() * 20);
	srand(tTiempo);

	for (x=0;x<15;x++)
		idMsg[x]= rand() % 25 + 97;

	idMsg[15]='\0';
	//strncpy(idMsg, idMsg, sizeof(idMsg));
}


char* getStringTipo(int tipo) {
	switch (tipo) {
	case NUEVO_PERSONAJE: return "NUEVO_PERSONAJE";
	break;
	case NUEVO_NIVEL: return "NUEVO_NIVEL";
	break;
	case PERSONAJE_CONECTADO: return "PERSONAJE_CONECTADO";
	break;
	case NIVEL_CONECTADO: return "NIVEL_CONECTADO";
	break;
	case CONECTAR_NIVEL: return "CONECTAR_NIVEL";
	break;
	case SOLICITUD_UBICACION: return "SOLICITUD_UBICACION";
	break;
	case SOLICITUD_RECURSO: return "SOLICITUD_RECURSO";
	break;
	case RECURSO_CONCEDIDO: return "RECURSO_CONCEDIDO";
	break;
	case RECURSO_DENEGADO: return "RECURSO_DENEGADO";
	break;
	case TURNO_CONCEDIDO: return "TURNO_CONCEDIDO";
	break;
	case FINALIZAR: return "FINALIZAR";
	break;
	case CAMBIOS_CONFIGURACION: return "CAMBIOS_CONFIGURACION";
	break;
	case UBICACION_RECURSO: return "UBICACION_RECURSO";
	break;
	case MOVIMIENTO_REALIZADO: return "MOVIMIENTO_REALIZADO";
	break;
	case RECURSO_INEXISTENTE: return "RECURSO_INEXISTENTE";
	break;
	case PLAN_NIVEL_FINALIZADO: return "PLAN_NIVEL_FINALIZADO";
	break;
	case PLAN_NIVELES_CONCLUIDO: return "PLAN_NIVELES_CONCLUIDO";
	break;
	case MUERTE_PERSONAJE: return "MUERTE_PERSONAJE";
	break;
	case MUERTE_PERSONAJE_XENEMIGO: return "MUERTE_PERSONAJE_XENEMIGO";
	break;
	case MUERTE_PERSONAJE_XRECOVERY: return "MUERTE_PERSONAJE_XRECOVERY";
	break;
	case NIVEL_INEXISTENTE: return "NIVEL_INEXISTENTE";
	break;
	case PERSONAJE_DESBLOQUEADO: return "PERSONAJE_DESBLOQUEADO";
	break;
	default: return "TIPO NO RECONOCIDO";
	}
}

/***********Señales *********/

void senialMurioHijo(int senial)
{
	/*

	switch (senial)

 case SIGCHLD:  puts("Murio un proceso hijo :-S...");
	  	             while(wait(NULL) > 0);
	  	             break;
	  default:       puts("la señal capturada no la tratamos");
	  	             break;
	 */
}

/*void senialSuspendido(int senial)
{


	switch (senial)
	{
	  case SIGCUSR1:  puts("Hay que suspender al proceso :-S..."); //definir las señales de usuario para la suspencion de los procesos
	  	             //enviar(SUSPENDIDO);
					//pi.estado=SUSPENDIDO;
	  	             break;
	  default:       puts("la señal capturada no la tratamos");
	  	             break;
	}
}*/
