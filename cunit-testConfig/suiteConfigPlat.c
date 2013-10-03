/*
 * suiteConfigPlat.c
 *
 *  Created on: 22/09/2013
 *      Author: arwen
 */

#include "suiteConfigPlat.h"


void test1 () {
	printf("Soy test1 y pruebo que configPlatPUERTO sea igual a 5000: ");

	levantarArchivoConfiguracionPlataforma();
	CU_ASSERT_EQUAL(configPlatPuerto(), 5000);
}

void test2 () {
	printf("Soy test2 y pruebo que configPlat->KOOPA sea igual a '/utnso/koopa': ");
	CU_ASSERT_STRING_EQUAL(configPlatKoopa(), "/utnso/koopa");
}

void test3 () {
	printf("Soy test3 y pruebo que configPlat->SCRIPT sea igual a '/utnso/script.sh': ");
	CU_ASSERT_STRING_EQUAL(configPlatScript(), "/utnso/script.sh");
}

void test4 () {
	printf("Soy test4 y pruebo que configPlat->LOG_PATH sea igual a '/tmp/plataforma.log': ");
	CU_ASSERT_STRING_EQUAL(configPlatLogPath(), "/tmp/plataforma.log");
}

void test5 () {
	printf("Soy test5 y pruebo que configPlat->NIVEL_LOG sea igual a 'DEBUG': ");
	CU_ASSERT_EQUAL(configPlatLogNivel(), LOG_LEVEL_DEBUG);
}

void test6 () {
	printf("Soy test6 y pruebo que configPlat->LOG_CONSOLA sea igual a 1: ");
	CU_ASSERT_EQUAL(configPlatLogConsola(), 1);
}

int suiteConfigPlat () {
	CU_initialize_registry();

	CU_pSuite prueba = CU_add_suite("Suite de prueba", NULL, NULL);
	CU_add_test(prueba, "uno", test1);
	CU_add_test(prueba, "dos", test2);
	CU_add_test(prueba, "tres", test3);
	CU_add_test(prueba, "cuatro", test4);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}
