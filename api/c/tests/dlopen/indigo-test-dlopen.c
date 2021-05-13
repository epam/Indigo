#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#define DLOPEN(a) LoadLibrary(a)
#define DLSYM(a, b) GetProcAddress(a, b)
#define DLERROR ""
#define DLCLOSE(a) FreeLibrary(a)
#define HANDLE HMODULE
#else
#include <dlfcn.h>
#define DLOPEN(a) dlopen(a, RTLD_GLOBAL | RTLD_NOW)
#define DLSYM(a, b) dlsym(a, b)
#define DLERROR dlerror()
#define DLCLOSE(a) dlclose(a)
#define HANDLE void*
#endif

typedef int (*INT_RET_STR)(const char*);
typedef int (*INT_RET)();
typedef const char* (*STR_RET_INT)(int);
typedef const char* (*STR_RET_VOID)(void);
typedef int (*INT_RET_INT_INT)(int, int);
typedef int (*INT_RET_STR_STR)(const char*, const char*);
typedef int (*INT_RET_STR_STR_STR)(const char*, const char*, const char*);
typedef int (*INT_RET_INT)(int);

/* Try to dynamically load library and check load status. */
HANDLE dlOpenWithCheck(const char* libraryPath)
{
    HANDLE handle = DLOPEN(libraryPath);
#ifdef _WIN32
    /* On Windows error code is returned by LoadLibrary() and can be between 0 and 31 */
    if (handle < (HANDLE)32)
    {
        printf("Error in LoadLibrary. Error code: %p\n", handle);
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

int main(int argc, char** argv)
{
    HANDLE indigoHandle;
    HANDLE indigoInChIHandle;
    HANDLE indigoRendererHandle;
    HANDLE bingoHandle;

    STR_RET_VOID indigoVersion;
    INT_RET_STR indigoLoadReactionFromString;
    INT_RET_STR indigoLoadMoleculeFromString;
    STR_RET_VOID indigoGetLastError;
    INT_RET_STR_STR indigoSetOption;
    INT_RET indigoWriteBuffer;
    INT_RET_INT_INT indigoRender;
    STR_RET_INT indigoInchiGetInchi;
    INT_RET_STR_STR_STR bingoCreateDatabaseFile;
    INT_RET_INT bingoCloseDatabase;
    STR_RET_VOID bingoVersion;

    int indigoTest = 0;
    int indigoInChITest = 0;
    int indigoRendererTest = 0;
    int bingoTest = 0;

    const char* indigoLibraryPath;
    const char* indigoInChILibraryPath;
    const char* indigoRendererLibraryPath;
    const char* bingoLibraryPath;

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
        if (strstr(argv[i], "bingo"))
        {
            bingoTest = 1;
            bingoLibraryPath = argv[i];
        }
    }
    /* Tests */
    if (indigoTest)
    {
        int r;
        /* Load Indigo */
        indigoHandle = dlOpenWithCheck(indigoLibraryPath);
        if (!indigoHandle)
        {
            printf("Cannot load %s\n", indigoLibraryPath);
            return 1;
        }
        printf("Indigo instance: %p\n", indigoHandle);
        /* Execute Indigo function */
        indigoVersion = (STR_RET_VOID)DLSYM(indigoHandle, "indigoVersion");
        printf("Indigo version: %s\n", indigoVersion());
        indigoLoadReactionFromString = (INT_RET_STR)DLSYM(indigoHandle, "indigoLoadReactionFromString");
        indigoGetLastError = (STR_RET_VOID)DLSYM(indigoHandle, "indigoGetLastError");
        r = indigoLoadReactionFromString("C");
        if (r < 0)
        {
            printf("Error handled: %s\n", indigoGetLastError());
        }
    }
    if (indigoInChITest)
    {
        int m;
        /* Load IndigoInChI */
        indigoInChIHandle = dlOpenWithCheck(indigoInChILibraryPath);
        if (!indigoInChIHandle)
        {
            printf("Cannot load %s\n", indigoInChILibraryPath);
            return 1;
        }
        printf("IndigoInChI address: %p\n", indigoInChIHandle);
        indigoInchiGetInchi = (STR_RET_INT)DLSYM(indigoInChIHandle, "indigoInchiGetInchi");
        indigoLoadMoleculeFromString = (INT_RET_STR)DLSYM(indigoHandle, "indigoLoadMoleculeFromString");
        m = indigoLoadMoleculeFromString("C");
        printf("indigoInChI InChI: %s\n", indigoInchiGetInchi(m));
    }
    if (indigoRendererTest)
    {
        int m, buf, res;
        /* Load IndigoRenderer */
        indigoRendererHandle = dlOpenWithCheck(indigoRendererLibraryPath);
        if (!indigoRendererHandle)
        {
            printf("Cannot load %s\n", indigoRendererLibraryPath);
            return 1;
        }
        printf("IndigoRenderer address: %p\n", indigoRendererHandle);
        indigoLoadMoleculeFromString = (INT_RET_STR)DLSYM(indigoHandle, "indigoLoadMoleculeFromString");
        indigoWriteBuffer = (INT_RET)DLSYM(indigoHandle, "indigoWriteBuffer");
        indigoRender = (INT_RET_INT_INT)DLSYM(indigoRendererHandle, "indigoRender");
        indigoSetOption = (INT_RET_STR_STR)DLSYM(indigoHandle, "indigoSetOption");
        indigoSetOption("render-output-format", "png");
        m = indigoLoadMoleculeFromString("C");
        buf = indigoWriteBuffer();
        res = indigoRender(m, buf);
        printf("indigoRender result: %d\n", res);
    }
    if (bingoTest)
    {
        int db;
        /* Load Bingo */
        bingoHandle = dlOpenWithCheck(bingoLibraryPath);
        if (!bingoHandle)
        {
            printf("Cannot load %s\n", bingoLibraryPath);
            return 1;
        }
        printf("Bingo address: %p\n", bingoHandle);
        bingoVersion = (STR_RET_VOID)DLSYM(bingoHandle, "bingoVersion");
        printf("Bingo version: %s\n", bingoVersion());
        bingoCreateDatabaseFile = (INT_RET_STR_STR_STR)DLSYM(bingoHandle, "bingoCreateDatabaseFile");
        bingoCloseDatabase = (INT_RET_INT)DLSYM(bingoHandle, "bingoCloseDatabase");
        db = bingoCreateDatabaseFile("test.db", "molecule", "");
        printf("Bingo database ID: %d\n", db);
        printf("Bingo close database status: %d\n", bingoCloseDatabase(db));
    }
    /* Close libraries */
    if (bingoTest)
    {
        DLCLOSE(bingoHandle);
    }
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
