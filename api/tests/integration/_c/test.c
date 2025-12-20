/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

// Based on https://pythonextensionpatterns.readthedocs.io/en/latest/debugging/debug_in_ide.html

#include <Python.h>

#include "test.h"

int import_call_execute(int argc, const char* argv[])
{
    int return_value = 0;
    PyObject* pModule = NULL;
    PyObject* pFunc = NULL;
    PyObject* pResult = NULL;

    Py_SetProgramName((wchar_t*)argv[0]);
    wchar_t** _argv = PyMem_Malloc(sizeof(wchar_t*) * argc);
    for (int i = 0; i < argc; i++)
    {
        wchar_t* arg = Py_DecodeLocale(argv[i], NULL);
        _argv[i] = arg;
    }
    size_t python_sys_path_len = strlen(PYTHON_SYS_PATH);
    Py_SetPath(Py_DecodeLocale(PYTHON_SYS_PATH, &python_sys_path_len));
    Py_Initialize();
    PySys_SetArgv(argc, _argv);
    PyMem_Free(_argv);
    pModule = PyImport_ImportModule("test");
    if (!pModule)
    {
        fprintf(stderr, "%s: Failed to load module 'test'", argv[0]);
        return_value = -3;
        goto except;
    }
    pFunc = PyObject_GetAttrString(pModule, "main");
    if (!pFunc)
    {
        fprintf(stderr, "%s: Can not find function 'main'", argv[0]);
        return_value = -4;
        goto except;
    }
    if (!PyCallable_Check(pFunc))
    {
        fprintf(stderr, "%s: Function \"%s\" is not callable\n", argv[0], argv[3]);
        return_value = -5;
        goto except;
    }
    pResult = PyObject_CallObject(pFunc, NULL);
    if (!pResult)
    {
        fprintf(stderr, "%s: Function call failed\n", argv[0]);
        return_value = -6;
        goto except;
    }
#ifdef DEBUG
    printf("%s: PyObject_CallObject() succeeded\n", argv[0]);
#endif
    assert(!PyErr_Occurred());
    goto finally;
except:
    assert(PyErr_Occurred());
    PyErr_Print();
finally:
    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);
    Py_XDECREF(pResult);
    Py_Finalize();
    return return_value;
}

int main(int argc, const char* argv[])
{
    return import_call_execute(argc, argv);
}
