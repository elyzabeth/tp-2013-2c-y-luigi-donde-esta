#include "comunicacion.h"


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

int recibir_header(int sock, header_t *header, fd_set *master/*por si se desconecta*/, int *seDesconecto)
{
	int ret;
	char *buffer = NULL;
	//char strAux[50];

	buffer = malloc(sizeof(header_t));
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
	}

	memcpy(header, buffer, sizeof(header_t)); /*ojo que el memcopy si lo haces afuera el primer parametro tiene que tener &*/
	/* Por ejemplo si la estructua no fuera por referencia y fuera local, debes hacer asi:
	memcpy(&header, buffer, sizeof(header_t));*/

	printf("sock: %d --- largo: %d ---- ", sock, header->largo_mensaje);
	printf("tipo: %d\n", header->tipo);
	free(buffer);

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
