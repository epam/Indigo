#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#define DLOPEN(a) LoadLibrary(a)
#define DLSYM(a, b) GetProcAddress(a, b)
#define DLERROR ""
#define DLCLOSE(a) FreeLibrary(a)
#define VOID_RET_STR FARPROC
#define HANDLE HMODULE
#else
#include <dlfcn.h>
#define DLOPEN(a) dlopen(a, RTLD_GLOBAL | RTLD_NOW)
#define DLSYM(a, b) dlsym(a, b)
#define DLERROR dlerror()
#define DLCLOSE(a) dlclose(a)
typedef const char* (*VOID_RET_STR)();
#define HANDLE void*
#endif

/* Try to dynamically load library and check load status. */
HANDLE dlOpenWithCheck(const char *libraryPath)
{	
	HANDLE handle = DLOPEN(libraryPath);
#ifdef _WIN32
	/* On Windows error code is returned by LoadLibrary() and can be between 0 and 31 */
	if (handle < 32)
	{
		printf("Error in LoadLibrary. Error code: %d\n", handle);
		return NULL;
	}
#else
	if (!handle)
	{
		printf("Error in dlopen: %s\n", dlerror());
		return NULL;
	}
#endif
	return handle;
}

int main(int argc, char **argv)
{
	HANDLE indigoHandle;
	HANDLE indigoInChIHandle;
	HANDLE indigoRendererHandle;
	VOID_RET_STR indigoVersion;
    VOID_RET_STR indigoLoadMoleculeFromString;
	VOID_RET_STR indigoWriteBuffer;
	VOID_RET_STR indigoRender;
	int indigoTest = 0;
	int indigoInChITest = 0;
	int indigoRendererTest = 0;
	const char *indigoLibraryPath;
	const char *indigoInChILibraryPath;
	const char *indigoRendererLibraryPath;
	int i = 0;
	/* Parse arguments and set variables*/
	for (i = 0; i < argc; i++)
	{
		if (strstr(argv[i], "indigo."))
		{
			indigoTest = 1;
			indigoLibraryPath = argv[i];
		}
		if (strstr(argv[i], "indigo-inchi"))
		{
			indigoInChITest = 1;
			indigoInChILibraryPath = argv[i];
		}
		if (strstr(argv[i], "indigo-renderer"))
		{
			indigoRendererTest = 1;
			indigoRendererLibraryPath = argv[i];
		}
	}
	/* Tests */
	if (indigoTest)
	{	
		/* Load Indigo */
		indigoHandle = dlOpenWithCheck(indigoLibraryPath);
		if (!indigoHandle)
		{
			printf("Cannot load %s\n", indigoLibraryPath);
			return 1;
		}
		printf("Indigo instance: %d\n", (int)indigoHandle);		
		/* Execute Indigo function */
		indigoVersion = DLSYM(indigoHandle, "indigoVersion");
		printf("Indigo address: %s\n", indigoVersion());
	}	
	if (indigoInChITest)
	{
		/* Load IndigoInChI */
		indigoInChIHandle = dlOpenWithCheck(indigoInChILibraryPath);
		if (!indigoInChIHandle)
		{
			printf("Cannot load %s\n", indigoInChILibraryPath);
			return 1;
		}
		printf("IndigoInChI address: %d\n", (int)indigoInChIHandle);
		/* TODO: Execute IndigoInChI function */
	}
	if (indigoRendererTest)
	{
		/* Load IndigoRenderer */
		indigoRendererHandle = dlOpenWithCheck(indigoRendererLibraryPath);
		if (!indigoRendererHandle)
		{
			printf("Cannot load %s\n", indigoRendererLibraryPath);
			return 1;
		}
		printf("IndigoRenderer address: %d\n", (int)indigoRendererHandle);
        indigoLoadMoleculeFromString = DLSYM(indigoHandle, "indigoLoadMoleculeFromString");
        indigoWriteBuffer = DLSYM(indigoHandle, "indigoWriteBuffer");
        indigoRender = DLSYM(indigoRendererHandle, "indigoRender");
        printf("indigoRender result: %d\n", indigoRender(indigoLoadMoleculeFromString("C"), indigoWriteBuffer()));
	}
	/* Close libraries */
	if (indigoRendererTest)
	{
		DLCLOSE(indigoRendererHandle);
	}
	if (indigoInChITest)
	{
		DLCLOSE(indigoInChIHandle);
	}
	if (indigoTest)
	{
		DLCLOSE(indigoHandle);
	}
	return 0;
}