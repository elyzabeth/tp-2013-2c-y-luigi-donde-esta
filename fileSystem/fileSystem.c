/*
 * fileSystem.c
 *
 *  Created on: 18/10/2013
 *      Author: elyzabeth
 */

#include "fileSystem.h"


void levantarHeader(int fd, char *HDR ) {

	HDR = (char*)mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);

	//LEO header
	memcpy(&HEADER, HDR, sizeof(GHeader));

	printf("\n\n--- HEADER: ----\n");
	printf("grasa: %s\n", HEADER.grasa);
	printf("version: %d\n", HEADER.version);
	printf("blk_bitmap: %d\n", HEADER.blk_bitmap);
	printf("size_bitmap: %d\n", HEADER.size_bitmap);

	printf("\n--- FIN HEADER: ----\n\n");

	munmap(HDR, 4096);

}

void mapearBitMap (int fd) {
	BITMAP = mmap(NULL, BLKSIZE*HEADER.size_bitmap, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, BLKSIZE * GHEADERBLOCKS);
	bitvector = bitarray_create(BITMAP, TAMANIODISCO);
}

void mapearTablaNodos(int fd) {
	int i;
	FNodo = mmap(NULL, BLKSIZE*GFILEBYTABLE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, BLKSIZE*(GHEADERBLOCKS+HEADER.size_bitmap));
	for (i=0; i < GFILEBYTABLE; i++) {
		NODOS[i] = (GFile*)(FNodo + i);
	}

}

void mapearDatos (int fd) {
	DATOS = mmap(NULL, TAMANIODISCO, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
}

//------------------------------------------

void imprimirTablaINodos () {
	int i;
	printf("\n\n------ Tabla Nodos --------\n");

	for(i=0; i < 1024; i++) {
		if (NODOS[i]->state) {
			printf("%d) bloque: %d - nombre: %s - state: %d - padre: %d - c_date: %llu - m_date: %llu \n", i, i+1, NODOS[i]->fname, NODOS[i]->state, NODOS[i]->parent_dir_block, NODOS[i]->c_date, NODOS[i]->m_date);
		}
		if (NODOS[i]->state == 1) {
			printf("\tArchivo: %s - tamanio: %d\n", NODOS[i]->fname, NODOS[i]->file_size);
			printf("\t\tPuntero 0 (bloque %d)\n", NODOS[i]->blk_indirect[0]);
		}
	}

	printf("\n------ FIN Tabla Nodos --------\n\n");

}


// -----------------------------------------------------------------------------------------
// FUNCIONES AUXILIARES
// ----------------------------------------------------------------------------------------------

void* getBlockAddress (ptrGBloque nroBloque) {
	return (DATOS + ( nroBloque * BLKSIZE ));
}

void initBlkIndirect (GFile *Nodo){
	int i;
	for (i = 0; i < BLKINDIRECT; i++)
		Nodo->blk_indirect[i] = 0;
}

void initBloque (ptrGBloque nroBloque) {
	//memset(DATOS + ( nroBloque * BLKSIZE), 0, BLKSIZE );
	memset( getBlockAddress(nroBloque) , 0, BLKSIZE );
}


GFile* reservarPrimerNodoLibre(int *posicion, uint8_t tipo) {
	pthread_mutex_lock (&mutexGrasaNodesTable);
	int i;
	GFile *Nodo = NULL;
	*posicion = -1;

	for(i=0; i < BLKDIRECT; i++) {
		if (NODOS[i]->state == BORRADO) {
			NODOS[i]->state = tipo;
			*posicion = i;
			Nodo = NODOS[i];
			break;
		}
	}
	pthread_mutex_unlock(&mutexGrasaNodesTable);
	return Nodo;
}

GFile* reservarNodoDirectorio(int *posicion) {
	return reservarPrimerNodoLibre(posicion, DIRECTORIO);
}

GFile* reservarNodoArchivo(int *posicion) {
	return reservarPrimerNodoLibre(posicion, ARCHIVO);
}


ptrGBloque reservarPrimerBloqueLibre() {
	pthread_mutex_lock (&mutexGrasaBitVector);
	int i;
	int startdatablk = GFILEBYTABLE + GHEADERBLOCKS + HEADER.size_bitmap;
	ptrGBloque bloque = 0;

	// bitmap va desde 0 a BITMAPBITS - 1 (0 representa al header and so on)
	for(i=startdatablk; i < BITMAPBITS; i++) {
		if ( bitarray_test_bit(bitvector, i) == 0 ) {
			bloque = i;
			break;
		}
	}
	bitarray_set_bit(bitvector, bloque);
	pthread_mutex_unlock(&mutexGrasaBitVector);

	return bloque;
}

ptrGBloque liberarBloque(ptrGBloque bloque) {
	pthread_mutex_lock (&mutexGrasaBitVector);
	bitarray_clean_bit(bitvector, bloque);
	pthread_mutex_unlock(&mutexGrasaBitVector);
	return bloque;
}

int liberarBloquesArchivo (GFile *Nodo) {
	ptrGBloque (*directBlk)[BLKDIRECT]= NULL;
	ptrGBloque nroBloque;
	int i=0, d=0, cont=0;

	while(Nodo->blk_indirect[i]) {
		directBlk = (ptrGBloque(*)[BLKDIRECT])(getBlockAddress(Nodo->blk_indirect[i]));
		d=0;

		while((*directBlk)[d]) {
			nroBloque = (*directBlk)[d++];
			log_debug(LOGGER, "liberarBloquesArchivo: Liberar bloque directo %ld %u ", nroBloque, nroBloque);
			liberarBloque(nroBloque);
			log_debug(LOGGER, "liberarBloquesArchivo: Liberar bloque directo %ld %u - estado: %d", nroBloque, nroBloque, bitarray_test_bit(bitvector, nroBloque));

			cont++;
		}
		nroBloque = Nodo->blk_indirect[i++];
		liberarBloque(nroBloque);
		log_debug(LOGGER, "liberarBloquesArchivo: Liberar bloque indirecto %ld %u ", nroBloque, nroBloque);
		log_debug(LOGGER, "liberarBloquesArchivo: Liberar bloque indirecto %ld %u - estado: %d", nroBloque, nroBloque, bitarray_test_bit(bitvector, nroBloque));
		cont++;
	}
	return cont;
}

GFile* getGrasaChildNode (const char *path, uint8_t tipo, int *posicion, int nroBloquePadre) {
	int i, encontrado=-1;
	const char *subpath = strrchr(path, '/');
	if (subpath == NULL)
		subpath = path;
	else
		subpath = subpath+1;

	for(i=0; i < BLKDIRECT && encontrado < 0; i++) {
		if (strcmp(subpath, NODOS[i]->fname) == 0 && NODOS[i]->state == tipo && NODOS[i]->parent_dir_block == nroBloquePadre) {
			encontrado = i;
		}
	}

	*posicion = encontrado;

	if (encontrado == -1)
		return NULL;

	return NODOS[encontrado];
}

GFile* getGrasaNode (const char *path, int *posicion) {
	int i, encontrado=-1;
	char *subpath = strrchr(path, '/');

	for(i=0; i < BLKDIRECT && encontrado < 0; i++) {
		if (strcmp(subpath+1, NODOS[i]->fname) == 0) {
			encontrado = i;
		}
	}

	*posicion = encontrado;

	if (encontrado == -1)
		return NULL;

	return NODOS[encontrado];
}


GFile* getGrasaNodeByType (const char *path, uint8_t tipo, int *posicion) {
	int i, encontrado=-1;
	char *subpath = strrchr(path, '/');

	for(i=0; i < 1024 && encontrado < 0; i++) {
		if (strcmp(subpath+1, NODOS[i]->fname) == 0 && NODOS[i]->state == tipo) {
			encontrado = i;
		}
	}

	*posicion = encontrado;

	if (encontrado == -1)
		return NULL;

	return NODOS[encontrado];
}

GFile* getGrasaDirNode (const char *path, int *posicion) {
	return getGrasaNodeByType(path, DIRECTORIO, posicion);
}

GFile* getGrasaFileNode (const char *path, int *posicion) {
	return getGrasaNodeByType(path, ARCHIVO, posicion);
}

GFile* buscarNodoPadre (const char *path, int *nroBloquePadre) {
	GFile *NodoPadre=NULL;
	int i=0;
	char *pathCopia = strdup(path);
	char **subpath = string_split(pathCopia, "/");
	char *tree[50];
	int level=0, pos;
	*nroBloquePadre = 0;

	void _getParentBlkNumber(char *dir) {
		tree[level++] = dir;
		log_debug(LOGGER, " %d ) buscarNodoPadre._getParentBlkNumber: %s - level: %d", level-1, dir, level);
	}

	string_iterate_lines(subpath, _getParentBlkNumber);

	if (level > 1) {

		for(i =0; i < level-1; i++) {
			NodoPadre = getGrasaChildNode(tree[i], DIRECTORIO, &pos, *nroBloquePadre);

			if (NodoPadre != NULL) {
				*nroBloquePadre = pos+1;
			} else {
				*nroBloquePadre = -1;
				break;
			}
		}
	}

	string_iterate_lines(subpath, (void*) free);
	free(subpath);
	free(pathCopia);

	return NodoPadre;
}


// NO se usa
void leerArchivo(int inodo, char *buf) {
	int i=0, cantAcopiar= BLKSIZE;
	uint32_t tamanioArchivo = NODOS[inodo]->file_size;
	char *aux = buf;

	while (i < BLKINDIRECT && NODOS[inodo]->blk_indirect[i] != 0 ) {
		cantAcopiar = tamanioArchivo<BLKSIZE?tamanioArchivo:BLKSIZE;
		memcpy(aux, DATOS+(NODOS[inodo]->blk_indirect[i] * BLKSIZE), cantAcopiar );
		aux += cantAcopiar;
		tamanioArchivo-=BLKSIZE;
	}
}

// Tampoco se usa
void copiarBloque (char *buf, int posicion, long long int nroBlkInd, long long int nroBlkDirect, long long int offsetBlkDirect, size_t size) {

	memcpy(blk_direct, DATOS+(NODOS[posicion]->blk_indirect[nroBlkInd]*BLKSIZE), BLKSIZE);
	log_debug(LOGGER, "3) blk_direct[%d]: %d",nroBlkDirect , blk_direct[nroBlkDirect]);

	memcpy(buf, DATOS+(blk_direct[nroBlkDirect] * BLKSIZE)+offsetBlkDirect, size);
}

size_t copiarABuffer (char *buf, GFile *Nodo, off_t offset, size_t size) {
	// off_t = long int
	// size_t = unsigned int

	off_t nroBlkInd, indblk_resto, nroBlkDirect, offsetBlkDirect;
	ptrGBloque (*directBlk)[BLKDIRECT]= NULL;
	//ptrGBloque directBlk1[BLKDIRECT]={0};
	size_t totalAcopiarDelBloque = 0;


	nroBlkInd = (offset / (BLKDIRECT * BLKSIZE));
	indblk_resto = offset % (BLKDIRECT * BLKSIZE);
	nroBlkDirect = (indblk_resto / BLKSIZE);
	offsetBlkDirect = (indblk_resto % BLKSIZE);

	totalAcopiarDelBloque = BLKSIZE - offsetBlkDirect;
	size=size<=totalAcopiarDelBloque?size:totalAcopiarDelBloque;

	log_debug(LOGGER, "\n\ncopiarABuffer\n*************\n offset: %lu \n indirect_block_number: %lu \n indblk_resto: %lu \n direct_block_number: %lu \n directblk_offset: %lu ", offset, nroBlkInd, indblk_resto, nroBlkDirect, offsetBlkDirect);
	log_debug(LOGGER, "- copiarABuffer: Nodo->blk_indirect[%d]: %d %u", nroBlkInd, Nodo->blk_indirect[nroBlkInd], Nodo->blk_indirect[nroBlkInd]);


	// COMO CASTEAR a:  un puntero a arreglo de 1024 de ptrGBloque => (ptrGBloque(*)[1024])
	// ---------------------------------------------------------------------------
	directBlk = (ptrGBloque(*)[BLKDIRECT])(DATOS+(Nodo->blk_indirect[nroBlkInd]*BLKSIZE));
	log_debug(LOGGER, "-- copiarABuffer: 1- directBlk[%d]: %d %u", nroBlkDirect , (*directBlk)[nroBlkDirect], (*directBlk)[nroBlkDirect]);

	if ((*directBlk)[nroBlkDirect] == 0){
		log_error(LOGGER, "ERROR en copiarABuffer el contenido del bloque directo[%ld - %u] es CERO!!", nroBlkDirect, nroBlkDirect);
		return -EFAULT; //Bad address
	}

	memcpy(buf, DATOS + ( ((*directBlk)[nroBlkDirect]) * BLKSIZE ) + offsetBlkDirect, size);

	return size;
}

size_t copiarDesdeBuffer (const char *buf, GFile *Nodo, off_t offset, size_t size) {
	// off_t = long int
	// size_t = unsigned int

	off_t nroBlkInd, indblk_resto, nroBlkDirect, offsetBlkDirect;
	ptrGBloque (*directBlk)[BLKDIRECT]= NULL;
	size_t totalAcopiarDelBloque = 0;

	//char *a = calloc(1, size+1);
	//memcpy(a, buf, size);
	//printf(" a: %s", a);
	//printf(" buffer: %s", buf);
	//free(a);

	nroBlkInd = (offset / (BLKDIRECT * BLKSIZE));
	indblk_resto = offset % (BLKDIRECT * BLKSIZE);
	nroBlkDirect = (indblk_resto / BLKSIZE);
	offsetBlkDirect = (indblk_resto % BLKSIZE);

	totalAcopiarDelBloque = BLKSIZE - offsetBlkDirect;
	size=size<=totalAcopiarDelBloque?size:totalAcopiarDelBloque;

	log_debug(LOGGER, "\n\n copiarDesdeBuffer\n*************\n offset: %lu %zu \n indirect_block_number: %lu %zu  \n indblk_resto: %lu %zu  \n direct_block_number: %lu %zu  \n directblk_offset: %lu %zu ", offset, offset, nroBlkInd, nroBlkInd, indblk_resto, indblk_resto, nroBlkDirect, nroBlkDirect, offsetBlkDirect, offsetBlkDirect);
	log_debug(LOGGER, "- copiarDesdeBuffer: Nodo->blk_indirect[%d]: %d %u", nroBlkInd, Nodo->blk_indirect[nroBlkInd], Nodo->blk_indirect[nroBlkInd]);


	// COMO CASTEAR a:  un puntero a arreglo de 1024 de ptrGBloque => (ptrGBloque(*)[1024])
	// ---------------------------------------------------------------------------
	//directBlk = (ptrGBloque(*)[BLKDIRECT])(DATOS+(Nodo->blk_indirect[nroBlkInd]*BLKSIZE));
	directBlk = (ptrGBloque(*)[BLKDIRECT])(getBlockAddress(Nodo->blk_indirect[nroBlkInd]));
	log_debug(LOGGER, "-- copiarDesdeBuffer: 1- directBlk[%d]: %d %u ", nroBlkDirect , (*directBlk)[nroBlkDirect], (*directBlk)[nroBlkDirect]);

	if ((*directBlk)[nroBlkDirect] == 0)
		(*directBlk)[nroBlkDirect] = reservarPrimerBloqueLibre();

	memcpy( getBlockAddress((*directBlk)[nroBlkDirect]) + offsetBlkDirect, buf, size);


	Nodo->file_size+=size;

	return size;
}



GFile* crearNuevoNodo(const char *path, struct fuse_file_info *fi){
	GFile *Nodo = NULL;
	char *subpath = strrchr(path, '/');
	int bloquePadre=0, pos=0;
	struct timeval now;
	ptrGBloque (*directBlk)[BLKDIRECT]= NULL;

	gettimeofday(&now, NULL);

	buscarNodoPadre(path, &bloquePadre);
	// TODO SI no existe el directorio del archivo lo creo??
	if (bloquePadre == -1) {
		Nodo = NULL;
		return Nodo;
	}

	Nodo = reservarNodoArchivo(&pos);

	//Nodo->state = ARCHIVO;
	strcpy(Nodo->fname, subpath+1);
	Nodo->c_date = now.tv_sec;
	Nodo->m_date = now.tv_sec;
	Nodo->file_size = 0;
	Nodo->parent_dir_block = bloquePadre;

	// TODO inicializar arreglo Nodo->blk_indirect ??
	// pongo en 0 cada posicion del arreglo
	initBlkIndirect (Nodo);
	// Busco primer bloque libre
	Nodo->blk_indirect[0] = reservarPrimerBloqueLibre();
	// "blanqueo" el bloque
	initBloque(Nodo->blk_indirect[0]);

	printf( "bloque libre: %u", Nodo->blk_indirect[0]);

	// TODO ya Reservo un bloque de datos????
	directBlk = (ptrGBloque(*)[BLKDIRECT])(getBlockAddress(Nodo->blk_indirect[0]));
	//directBlk[0] = reservarPrimerBloqueLibre();

	imprimirTablaINodos();

	return Nodo;
}

// FIN FUNCIONES AUXILIARES
// ----------------------------------------------------------------------------------------------

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la metadata de un archivo/directorio. Esto puede ser tamaño, tipo,
 * permisos, dueño, etc ...
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		stbuf - Esta esta estructura es la que debemos completar
 *
 * 	@RETURN
 * 		O archivo/directorio fue encontrado. -ENOENT archivo/directorio no encontrado
 */
static int grasa_getattr(const char *path, struct stat *stbuf) {
	log_debug(LOGGER, "grasa_getattr: %s", path);
	int res = 0;
	int i=0, encontrado=-1;
	char *subpath = strrchr(path, '/');

	memset(stbuf, 0, sizeof(struct stat));

	//Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje
	if (strcmp(path, "/") == 0) {
		encontrado = i;
		stbuf->st_mode = 0755|S_IFDIR;
		stbuf->st_nlink = 2;
		stbuf->st_uid = 1001;
		stbuf->st_gid = 1001;

	} else {

		for(i=0; i < 1024; i++) {

			if (strcmp(subpath+1, NODOS[i]->fname) == 0 && NODOS[i]->state != 0) {

				encontrado = i;

				stbuf->st_ctim.tv_sec = NODOS[i]->c_date;
				stbuf->st_mtim.tv_sec = NODOS[i]->m_date;
				stbuf->st_size = NODOS[i]->file_size;
				stbuf->st_uid = 1001;
				stbuf->st_gid = 1001;

				if (NODOS[i]->state == 2) {
					stbuf->st_mode = S_IFDIR | 0755;
					stbuf->st_nlink = 2;
					//stbuf->st_size = BLKLEN;

				} else if (NODOS[i]->state == 1) {
					stbuf->st_mode = S_IFREG | 0777;
					stbuf->st_nlink = 1;
				}
			}
		}
	}

	if (encontrado==-1)
		res = -ENOENT;

	return res;
}


int grasa_access (const char *path, int mask) {
	log_debug(LOGGER, "grasa_access: %s", path);
	return 0;
}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la lista de archivos o directorios que se encuentra dentro de un directorio
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		buf - Este es un buffer donde se colocaran los nombres de los archivos y directorios
 * 		      que esten dentro del directorio indicado por el path
 * 		filler - Este es un puntero a una función, la cual sabe como guardar una cadena dentro
 * 		         del campo buf
 *
 * 	@RETURN
 * 		O directorio fue encontrado. -ENOENT directorio no encontrado
 */
static int grasa_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	log_debug(LOGGER, "grasa_readdir: %s", path);
	(void) offset;
	(void) fi;
	int i, encontrado=-1;
	struct stat *stbuf;
	uint32_t directorio = 0;
	char *subpath = strrchr(path, '/');

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	encontrado =1;

	if (strcmp(path, "/")==0) {
		directorio = 0;
	} else {
		for(i=0; i < 1024; i++) {
			if (strcmp(subpath+1, NODOS[i]->fname) == 0 && NODOS[i]->state != 0) {
				encontrado = i;
				directorio = i+1;
			}
		}
	}

	if (encontrado==-1)
		return -ENOENT;

	for(i=0; i < 1024; i++) {
		if (NODOS[i]->parent_dir_block == directorio && NODOS[i]->state != 0) {
			stbuf = malloc(sizeof(struct stat));
			memset(stbuf, 0, sizeof(struct stat));
			if (NODOS[i]->state == 2) {
				stbuf->st_mode = 0755|S_IFDIR;
				stbuf->st_nlink = 2;
				stbuf->st_size = BLKSIZE;
				stbuf->st_uid = 1001;
				stbuf->st_gid = 1001;
			} else if (NODOS[i]->state == 1) {
				stbuf->st_mode = 0444 | S_IFREG;
				stbuf->st_nlink = 1;
				stbuf->st_size = NODOS[i]->file_size;
				stbuf->st_uid = 1001;
				stbuf->st_gid = 1001;
			}

			filler(buf, NODOS[i]->fname, stbuf, 0);
		}
	}


	return 0;
}


static int grasa_mkdir (const char *path, mode_t mode) {
	log_debug(LOGGER, "grasa_mkdir: %s", path);
	GFile *Nodo = NULL;
	//GFile *nodoPadre = NULL;
	int posicion, nroBloquePadre=0;
	char *subpath = strrchr(path, '/');

	// crear NODO que represente al directorio nuevo!!!!
	buscarNodoPadre(path, &nroBloquePadre);

	if (nroBloquePadre == -1) {
		return -ENOENT;
	}

	Nodo = reservarNodoDirectorio(&posicion);
	Nodo->parent_dir_block = nroBloquePadre;
	strncpy(Nodo->fname, subpath+1, GFILENAMELENGTH);
	Nodo->file_size = BLKSIZE;

	imprimirTablaINodos();
	return 0;

}

int grasa_rmdir (const char *path) {
	log_debug(LOGGER, "grasa_rmdir: %s", path);
	int posicion;
	GFile *Nodo = getGrasaDirNode(path, &posicion);

	if (posicion<0){
		return -ENOENT;
	}
	Nodo->state = BORRADO;

	return 0;
}

int grasa_unlink (const char *path) {
	log_debug(LOGGER, "grasa_unlink: %s", path);
	int posicion;
	GFile *Nodo = getGrasaFileNode(path, &posicion);

	if (posicion<0){
		return -ENOENT;
	}
	Nodo->state = BORRADO;
	liberarBloquesArchivo(Nodo);

	imprimirTablaINodos();

	return 0;
}
/** File open operation
 *
 * No creation (O_CREAT, O_EXCL) and by default also no
 * truncation (O_TRUNC) flags will be passed to open(). If an
 * application specifies O_TRUNC, fuse first calls truncate()
 * and then open(). Only if 'atomic_o_trunc' has been
 * specified and kernel version is 2.6.24 or later, O_TRUNC is
 * passed on to open.
 *
 * Unless the 'default_permissions' mount option is given,
 * open should check if the operation is permitted for the
 * given flags. Optionally open may also return an arbitrary
 * filehandle in the fuse_file_info structure, which will be
 * passed to all file operations.
 *
 * Changed in version 2.2
 */
static int grasa_open(const char *path, struct fuse_file_info *fi)
{
//	GFile *fileNode;
//	int posicion = -1;
//
//	fileNode = getGrasaFileNode(path, &posicion);
//
//	if ( posicion < 0 ) {
//		return -ENOENT;
//	}
//

	log_debug(LOGGER, "grasa_open: %s", path);

//	if ((fi->flags & 3) == O_RDONLY )
//		return -EACCES;

	return 0;
}

static int grasa_read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	log_debug(LOGGER, "grasa_read: %s", path);
	// NOTA: off_t = long int
	// size_t = unsigned int
	//long int indirect_block_number, indblk_resto, direct_block_number, directblk_offset;

	(void) fi;
	int posicion;
	//size_t len;
	GFile *fileNode;
	size_t copiado = 0;

	fileNode = getGrasaFileNode(path, &posicion);

	if (posicion<0){
		return -ENOENT;
	}

	//len = fileNode->file_size;

	log_debug(LOGGER, "\n\ngrasa_read: LLega: offset %ld - size: %u - size: %zu", offset, size, size);

	while( copiado < size) {
		log_debug(LOGGER, "\n\ngrasa_read: offset %ld - size: %u - size: %zu - copiado: %u - copiado: %zu", offset, size, size, copiado, copiado);
		//copiado = copiarBloque(buf, posicion, indirect_block_number, direct_block_number, directblk_offset, size);
		copiado += copiarABuffer(buf+copiado, fileNode, offset+copiado, size - copiado);
	}

	return size;

}

int grasa_truncate (const char *path, off_t offset) {
	log_debug(LOGGER, "grasa_truncate: %s", path);
	return 0;
}

int grasa_flush (const char *path, struct fuse_file_info *fi) {
	log_debug(LOGGER, "grasa_flush: %s", path);
	return 0;
}

int grasa_setxattr(const char* path, const char* name, const char* value, size_t size, int flags) {
	log_debug(LOGGER, "grasa_setxattr: %s - %s - %s", path, name, value);
	return 0;
}

static int grasa_mknod (const char *path, mode_t mode, dev_t dev){
	log_debug(LOGGER, "grasa_mknod: %s", path);
	return 0;
}

static int grasa_utimens(const char* path, const struct timespec ts[2]) {
	log_debug(LOGGER, "grasa_utimens: %s", path);
	return 0;
}

int grasa_rename (const char *path, const char *newPath){
	log_debug(LOGGER, "grasa_rename: %s -> %s", path, newPath);
	GFile *Nodo;
	int bloquePadre, posicion;
	char *subpath = strrchr(newPath, '/');

	Nodo = getGrasaNode(path, &posicion);
	if (Nodo == NULL) {
		return -ENOENT;
	}

	buscarNodoPadre(newPath, &bloquePadre);
	if (bloquePadre == -1) {
		Nodo = NULL;
		return -ENOENT;
	}

	Nodo->parent_dir_block = bloquePadre;
	strncpy(Nodo->fname, subpath+1, GFILENAMELENGTH);

	return 0;
}

int grasa_create (const char *path, mode_t mode, struct fuse_file_info *fi) {
	log_debug(LOGGER, "grasa_create: %s", path);
	crearNuevoNodo(path, fi);
	//int bloque_padre = 0;

	return 0;
}


static int grasa_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	pthread_mutex_lock (&mutexGrasaWrite);
	int posicion;
	GFile *fileNode;
	size_t copiado = 0;

	log_debug(LOGGER, "grasa_write: %s", path);

	fileNode = getGrasaFileNode(path, &posicion);

	// El archivo no existe -> Crear uno nuevo nodo.
	if ( posicion < 0 ) {
		//fileNode = crearNuevoNodo(path,fi);
		return -ENOENT;
	}


	// copiarDesdeBuffer(buf, fileNode, offset, size);

	while( copiado < size) {
		log_debug(LOGGER, "\n\n grasa_write: offset %ld - size: %u - size: %zu - copiado: %u - copiado: %zu", offset, size, size, copiado, copiado);
		//copiado += copiarABuffer(buf+copiado, fileNode, offset+copiado, size - copiado);
		copiado += copiarDesdeBuffer(buf+copiado, fileNode, offset+copiado, size - copiado);
	}


	pthread_mutex_unlock (&mutexGrasaWrite);

	return size;
}

/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 */

static struct fuse_operations grasa_oper = {

		.create = grasa_create,
		.getattr = grasa_getattr,
		.readdir = grasa_readdir,
		.read = grasa_read,
		.mkdir = grasa_mkdir,
		.rmdir = grasa_rmdir,
		.unlink = grasa_unlink,
		.open=grasa_open,
		.write = grasa_write,
		.mknod=grasa_mknod,
		.truncate = grasa_truncate,
		.flush = grasa_flush,
		.setxattr = grasa_setxattr,
		.utimens = grasa_utimens,
		.rename = grasa_rename
		//.destroy = grasa_destroy
};

int main (int argc, char**argv) {

	int ret, fd = -1;
	char *HDR=NULL;

	pthread_mutex_init (&mutexGrasaWrite, NULL);
	pthread_mutex_init (&mutexGrasaBitVector, NULL);
	pthread_mutex_init (&mutexGrasaNodesTable, NULL);

	LOGGER = log_create("fileSystem.log", "FILESYSTEM", 1, LOG_LEVEL_DEBUG);
	log_info(LOGGER, "INICIALIZANDO FILESYSTEM ");

	if ((fd = open("./disk.bin", O_RDWR, 0777)) == -1)
		err(1, "fileSystem main: error al abrir ./disk.bin (open)");

	levantarHeader(fd, HDR);
	mapearTablaNodos(fd);
	mapearDatos(fd);
	mapearBitMap(fd);

	// printf("bitarray_test_bit %d: %d\n", 1027, bitarray_test_bit(bitvector, 1027));
	int i;
	bitarray_set_bit(bitvector, 0);
	bitarray_clean_bit(bitvector, 1);

	for (i = 2552; i <= BITMAPBITS; i++)
		printf("\nbitarray_test_bit %d: %d", i, bitarray_test_bit(bitvector, i));

	bitarray_set_bit(bitvector, BITMAPBITS-1);

	imprimirTablaINodos();


	 struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	// Esta es la funcion principal de FUSE, es la que se encarga
	// de realizar el montaje, comuniscarse con el kernel, delegar todo
	// en varios threads
	ret = fuse_main(args.argc, args.argv, &grasa_oper, NULL);

	close(fd);
	munmap(BITMAP, BLKSIZE*HEADER.size_bitmap);
	munmap(FNodo, BLKSIZE*GFILEBYTABLE);
	munmap(DATOS, TAMANIODISCO);

	pthread_mutex_destroy(&mutexGrasaWrite);
	pthread_mutex_destroy(&mutexGrasaBitVector);
	pthread_mutex_destroy(&mutexGrasaNodesTable);

	return ret;
}
