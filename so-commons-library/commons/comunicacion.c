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
     	printf("Error al recibir datos\n\n");
	    return ERROR;
	  }
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
	size_t dirLong;
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

	printf("Espero recibir t_nivel (%u)", sizeof(t_nivel));
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
	close(desc);
	FD_CLR(desc, listaDesc);

	if (desc == *maxDesc)
	{
		*maxDesc = *maxDesc-1;
	}
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
