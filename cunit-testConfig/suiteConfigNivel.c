/*
 * suiteConfigNivel.c
 *
 *  Created on: 22/09/2013
 *      Author: arwen
 */

#include "suiteConfigNivel.h"

void testNivel1 () {
	printf("Soy testNivel1 y pruebo que configNivel->NombreNivel sea igual a 'nivel1': ");
	levantarArchivoConfiguracionNivel();
	CU_ASSERT_STRING_EQUAL(configNivelNombre(), "nivel1");
}

void testNivel2 () {
	printf("Soy testNivel2 y pruebo que configNivel->Plataforma sea igual a '192.168.0.100:5000': ");
	CU_ASSERT_STRING_EQUAL(configNivelPlataforma(), "192.168.0.100:5000");
}

void testNivel3 () {
	printf("Soy testNivel3 y pruebo que configNivel->PlataformaIP sea igual a '192.168.0.100': ");
	CU_ASSERT_STRING_EQUAL(configNivelPlataformaIp(), "192.168.0.100");
}

void testNivel4 () {
	printf("Soy testNivel4 y pruebo que configNivel->PlataformaPuerto sea igual a 5000: ");
	CU_ASSERT_EQUAL(configNivelPlataformaPuerto(), 5000);
}

void testNivel5 () {
	printf("Soy testNivel5 y pruebo que configNivel->TiempoChequeoDeadlock sea igual a 10000: ");
	CU_ASSERT_EQUAL(configNivelTiempoChequeoDeadlock(), 10000);
}

void testNivel6 () {
	printf("Soy testNivel6 y pruebo que configNivel->Recovery sea igual a 1: ");
	CU_ASSERT_EQUAL(configNivelRecovery(), 1);
}

void testNivel7 () {
	printf("Soy testNivel7 y pruebo que configNivel->Enemigos sea igual a 3: ");
	CU_ASSERT_EQUAL(configNivelEnemigos(), 3);
}

void testNivel8 () {
	printf("Soy testNivel8 y pruebo que configNivel->SleepEnemigos sea igual a 2000: ");
	CU_ASSERT_EQUAL(configNivelSleepEnemigos(), 2000);
}

void testNivel9 () {
	printf("Soy testNivel9 y pruebo que configNivel->Algoritmo sea igual a 'RR' %s: ", configNivelAlgoritmo());
	CU_ASSERT_STRING_EQUAL(configNivelAlgoritmo(), "RR");
}

void testNivel10 () {
	printf("Soy testNivel10 y pruebo que configNivel->Quantum sea igual a 3: ");
	CU_ASSERT_EQUAL(configNivelQuantum(), 3);
}

void testNivel11 () {
	printf("Soy testNivel11 y pruebo que configNivel->Retardo sea igual a 500: ");
	CU_ASSERT_EQUAL(configNivelRetardo(), 500);
}

void testNivel12 () {
	printf("Soy testNivel12 y pruebo que configNivel->LogPath sea igual a '/tmp/nivel.log': ");
	CU_ASSERT_STRING_EQUAL(configNivelLogPath(), "/tmp/nivel.log");
}

void testNivel13 () {
	printf("Soy testNivel13 y pruebo que configNivel->LogNivel sea igual a 'DEBUG' %d: ", configNivelLogNivel());
	CU_ASSERT_EQUAL(configNivelLogNivel(), LOG_LEVEL_DEBUG);
}

void testNivel14 () {
	printf("Soy testNivel14 y pruebo que configNivel->LogPantalla sea igual a 1: ");
	CU_ASSERT_EQUAL(configNivelLogConsola(), 1);
}

int suiteConfigNivel () {
	CU_initialize_registry();

	CU_pSuite prueba = CU_add_suite("Suite de prueba Nivel ", NULL, NULL);
	CU_add_test(prueba, "1", testNivel1);
	CU_add_test(prueba, "2", testNivel2);
	CU_add_test(prueba, "3", testNivel3);
	CU_add_test(prueba, "4", testNivel4);
	CU_add_test(prueba, "5", testNivel5);
	CU_add_test(prueba, "6", testNivel6);
	CU_add_test(prueba, "7", testNivel7);
	CU_add_test(prueba, "8", testNivel8);
	CU_add_test(prueba, "9", testNivel9);
	CU_add_test(prueba, "10", testNivel10);
	CU_add_test(prueba, "11", testNivel11);
	CU_add_test(prueba, "12", testNivel12);
	CU_add_test(prueba, "13", testNivel13);
	CU_add_test(prueba, "14", testNivel14);
	//TODO falta agregar caso configRecurso y configRecursos

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	destruirConfigNivel();
	return CU_get_error();
}
