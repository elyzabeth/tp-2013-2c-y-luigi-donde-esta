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

	printf("--- HEADER: ----\n");
	printf("grasa: %s:\n", HEADER.grasa);
	printf("version: %d:\n", HEADER.version);
	printf("blk_bitmap: %d:\n", HEADER.blk_bitmap);
	printf("size_bitmap: %d:\n", HEADER.size_bitmap);

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


GFile* getGrasaNode (const char *path, uint8_t tipo, int *posicion) {
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
	return getGrasaNode(path, DIRECTORIO, posicion);
}

GFile* getGrasaFileNode (const char *path, int *posicion) {
	return getGrasaNode(path, ARCHIVO, posicion);
}

void leerArchivo(int inodo, char *buf) {
	int i=0, cantAcopiar= BLKSIZE;
	uint32_t tamanioArchivo = NODOS[inodo]->file_size;
	char*aux = buf;

	while (i < 1024 && NODOS[inodo]->blk_indirect[i] != 0 ) {
		cantAcopiar = tamanioArchivo<BLKSIZE?tamanioArchivo:BLKSIZE;
		memcpy(aux, DATOS+(NODOS[inodo]->blk_indirect[i] * BLKSIZE), cantAcopiar );
		aux += cantAcopiar;
		tamanioArchivo-=BLKSIZE;
	}
}

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

	log_info(LOGGER, "\n\ncopiarABuffer\n*************\n offset: %lu \n indirect_block_number: %lu \n indblk_resto: %lu \n direct_block_number: %lu \n directblk_offset: %lu ", offset, nroBlkInd, indblk_resto, nroBlkDirect, offsetBlkDirect);
	log_debug(LOGGER, "- copiarABuffer: Nodo->blk_indirect[%d]: %d", nroBlkInd, Nodo->blk_indirect[nroBlkInd]);


	//memcpy(directBlk1, DATOS+(Nodo->blk_indirect[nroBlkInd]*BLKSIZE), BLKSIZE);
	//log_debug(LOGGER, "-- copiarABuffer: 2- directBlk[%d]: %u", nroBlkDirect , directBlk1[nroBlkDirect]);
	//memcpy(buf, DATOS+( (directBlk1[nroBlkDirect]) * BLKSIZE ) + offsetBlkDirect, size);

	directBlk = (ptrGBloque(*)[BLKDIRECT])(DATOS+(Nodo->blk_indirect[nroBlkInd]*BLKSIZE));
	log_debug(LOGGER, "-- copiarABuffer: 1- directBlk[%d]: %d", nroBlkDirect , (*directBlk)[nroBlkDirect]);

	memcpy(buf, DATOS + ( ((*directBlk)[nroBlkDirect]) * BLKSIZE ) + offsetBlkDirect, size);

	return size;
}


GFile* buscarPrimerNodoLibre(int *posicion){
	int i;
	GFile *Nodo = NULL;
	*posicion = -1;

	for(i=0; i < 1024; i++) {
		if (NODOS[i]->state == 0) {
			*posicion = i;
			Nodo = NODOS[i];
			break;
		}
	}
	return Nodo;
}

GFile* crearNuevoNodo(const char *path, struct fuse_file_info *fi){
	int posicion;
	GFile *Nodo;
	GFile *NodoPadre;
	char *pathCopia = strdup(path);
	char **subpath = string_split(pathCopia, "/");

	Nodo = buscarPrimerNodoLibre(&posicion);
	int i=0;

	void _getParentBlkNumber(char *dir) {
		int pos;
		NodoPadre = getGrasaDirNode(dir, &pos);
		log_debug(LOGGER, " %d )crearNuevoNodo: %s", i++, dir);
	}

	string_iterate_lines(subpath, _getParentBlkNumber);


	string_iterate_lines(subpath, (void*) free);
	free(subpath);
	free(pathCopia);

	return Nodo;
}


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
					stbuf->st_mode = S_IFREG | 0644;
					stbuf->st_nlink = 1;
				}
			}
		}
	}

	if (encontrado==-1)
		res = -ENOENT;

	return res;
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
	return 0;
}

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

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int grasa_read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	// NOTA: off_t = long int
	// size_t = unsigned int
	//long int indirect_block_number, indblk_resto, direct_block_number, directblk_offset;

	(void) fi;
	int posicion;
	size_t len;
	GFile *fileNode;
	size_t copiado = 0;

	fileNode = getGrasaFileNode(path, &posicion);

	if (posicion<0){
		return -ENOENT;
	}

	len = fileNode->file_size;

	log_info(LOGGER, "\n\ngrasa_read: LLega: offset %ld - size: %u - size: %zu", offset, size, size);

	while( copiado < size) {
		log_info(LOGGER, "\n\ngrasa_read: offset %ld - size: %u - size: %zu - copiado: %u - copiado: %zu", offset, size, size, copiado, copiado);
		//copiado = copiarBloque(buf, posicion, indirect_block_number, direct_block_number, directblk_offset, size);
		copiado += copiarABuffer(buf+copiado, fileNode, offset+copiado, size - copiado);
	}

	return size;

}


static int grasa_mknod (const char *path, mode_t mode, dev_t dev){
	log_debug(LOGGER, "grasa_mknod: %s", path);
	return 0;
}

int grasa_create (const char *path, mode_t mode, struct fuse_file_info *fi) {
	log_debug(LOGGER, "grasa_create: %s", path);
	return 0;
}

//int grasa_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
static int grasa_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	pthread_mutex_lock (&mutexGrasaWrite);
	int posicion;
	GFile *fileNode;
	//size_t copiado = 0;

	log_debug(LOGGER, "grasa_write: %s", path);

	fileNode = getGrasaFileNode(path, &posicion);

	// El archivo no existe -> Crear uno nuevo nodo.
	if ( posicion < 0 ) {
		fileNode = crearNuevoNodo(path,fi);
	}
	// TODO

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
		.open=grasa_open,
		.write = grasa_write,
		.mknod=grasa_mknod
		//.destroy = grasa_destroy
};

int main (int argc, char**argv) {

	int ret, fd = -1;
	char *HDR=NULL;
	int i;

	pthread_mutex_init (&mutexGrasaWrite, NULL);

	LOGGER = log_create("fileSystem.log", "FILESYSTEM", 1, LOG_LEVEL_DEBUG);
	log_info(LOGGER, "INICIALIZANDO FILESYSTEM ");

	if ((fd = open("./disk.bin", O_RDWR, 0777)) == -1)
		err(1, "fileSystem main: error al abrir ./disk.bin (open)");

	levantarHeader(fd, HDR);
	mapearTablaNodos(fd);
	mapearDatos(fd);
	mapearBitMap(fd);

	printf("bitarray_test_bit %d: %d\n", 1027, bitarray_test_bit(bitvector, 1027));

	puts("-----Tabla Nodos-------");

	for(i=0; i < 1024; i++) {
		if (NODOS[i]->state) {
			printf("%d) bloque: %d - nombre: %s - state: %d - padre: %d - c_date: %llu - m_date: %llu \n", i, i+1, NODOS[i]->fname, NODOS[i]->state, NODOS[i]->parent_dir_block, NODOS[i]->c_date, NODOS[i]->m_date);
		}
		if (NODOS[i]->state == 1) {
			printf("\tArchivo: %s - tamanio: %d\n", NODOS[i]->fname, NODOS[i]->file_size);
			printf("\t\tPuntero 0 (bloque %d)\n", NODOS[i]->blk_indirect[0]);
		}
	}

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

	return ret;
}
