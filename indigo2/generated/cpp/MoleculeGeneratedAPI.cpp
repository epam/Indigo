// Java
JNIEXPORT long JNICALL Java_com_dc_indigo_molecule_lib_createFromMolFile(JNIEnv* env, jobject thisObject, jstring string)
{
    std::string str = (*env)->GetStringUTFChars(env, string, NULL);
    (*env)->ReleaseStringUTFChars(env, string, name);
    return API::addObject(Molecule::createFromMolFile(str));
}

JNIEXPORT void JNICALL Java_com_dc_indigo_molecule_lib_aromatize(JNIEnv* env, jobject thisObject, long handle)
{
    API::getObject<Molecule>(handle)->aromatize();
}

JNIEXPORT jstring JNICALL Java_com_dc_indigo_molecule_lib_molfile(JNIEnv* env, jobject thisObject, long handle)
{
    return (*env)->NewStringUTF(env, API::getObject<Molecule>(handle)->molfile().c_str());
}

// Python
DLLEXPORT long Python_com_dc_indigo_molecule_lib_createFromMolFile(const char* string)
{
    std::string str = string;
    return API::addObject(Molecule::createFromMolFile(str));
}

DLLEXPORT void Python_com_dc_indigo_molecule_lib_aromatize(long handle)
{
    API::getObject<Molecule>(handle)->aromatize();
}


DLLEXPORT const char* Python_com_dc_indigo_molecule_lib_molfile(long handle)
{
    std::string str = API::getObject<Molecule>(handle)->molfile();
    char* ret = malloc(str.size() + 1); // will be freed by generated Python code - see Molecule.py
    strncpy(ret, str.c_str(), sizeof(ret));
    return ret;
}

