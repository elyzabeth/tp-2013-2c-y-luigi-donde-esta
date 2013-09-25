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
	printf("Soy test3 y pruebo que configPlat->NIVEL_LOG sea igual a 'DEBUG': ");
	CU_ASSERT_STRING_EQUAL(configPlatNivelLog(), "DEBUG");
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
