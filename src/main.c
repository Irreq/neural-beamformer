/*******************************************************************************
 * Title                 :   Main entry point for Beamformer
 * Filename              :   main.c
 * Author                :   Irreq
 * Origin Date           :   2022-12-29 12:57:41.103667
 * Version               :   1.0.0
 * Compiler              :
 * Target                :
 * Notes                 :   None
 *******************************************************************************/

/*************** MODULE REVISION LOG ******************************************
 *
 *    Date    Software Version    Initials   Description
 *  2022-12-29 12:57:41.103667
 *
 *******************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/

#include <stdio.h>

#include "Python.h"
#include "cy_api.h" // Will be created upon build

/******************************************************************************
 * Module Constants
 *******************************************************************************/

int main(int argc, char *argv[])
{
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);

    if (program == NULL)
    {
        exit(1);
    }
    Py_SetProgramName(program);

    PyImport_AppendInittab("cy_api", PyInit_cy_api);
    Py_Initialize();
    PyImport_ImportModule("cy_api");

    // Cython entrypoint
    handler();

    Py_Finalize();
    return 0;
}