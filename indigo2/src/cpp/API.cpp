JNIEXPORT void JNICALL Java_com_dc_indigo_molecule_lib_IndigoObject_release(JNIEnv* env, jobject thisObject, long handle)
{
    API::releaseObject(handle);
}

DLLEXPORT void Python_com_dc_indigo_object_lib_release(long handle)
{
    API::releaseObject(handle);
}


class API
{
public:
    static template <class T> long addObject(std::shared_ptr<T> obj)
    {
        auto handle = ++freeHandle;
        objects[handle] = obj;
        return handle;
    }

    static template <class T> std::shared_ptr<T> getObject(long handle)
    {
        return objects[handle];
    }

    static void releaseObject(long handle)
    {
        objects.erase(handle);
    }

private:
    static std::unordered_map<int, std::shared_ptr<void> obj> objects;
    static long freeHandle = 0;
};

