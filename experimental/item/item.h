#ifndef __item__
#define __item__

#include "../reflection/reflection.h"

#include <mutex>
#include <string>
#include <vector>

#define GETTER(TYPE, NAME) virtual TYPE NAME() const      // getter
#define SETTER(TYPE, NAME) virtual void NAME(const TYPE&) // setter
#define GETSETTER(TYPE, NAME) virtual TYPE NAME() const
virtual void NAME(const TYPE&)

#define GETTER_MODEL_IMPL(CLASS, TYPE, NAME)                                                                                                                   \
    TYPE CLASS::NAME()                                                                                                                                         \
    {                                                                                                                                                          \
        return NAME;                                                                                                                                     \
    }
#define SETTER_MODEL_IMPL(CLASS, TYPE, NAME)                                                                                                                   \
    void CLASS::NAME(const TYPE& _##NAME)                                                                                                                         \
    {                                                                                                                                                          \
        NAME = _##NAME;                                                                                                                                     \
    }

#define GETSETTER_MODEL_IMPL(CLASS, TYPE, NAME)                                                                                                                \
    TYPE CLASS::NAME()                                                                                                                                         \
    {                                                                                                                                                          \
        return NAME;                                                                                                                                     \
    }                                                                                                                                                          \
    void CLASS::NAME(const TYPE& _##NAME)                                                                                                                      \
    {                                                                                                                                                          \
        NAME = _##NAME;                                                                                                                                  \
    }

    namespace indigo2
{
    typedef id_t int;

    class Item // interface
    {
    public:
        GETTER(id_t, ID) = 0; // id to use as ref when serialized

        GETSETTER(std::string, name) = 0; // human-friendly name
        GETSETTER(std::string, uri) = 0;  // globally unique URI (can be an URL for loading of the item from remote service)

        virtual const std::lock_guard<std::mutex> lock() = 0;

    protected:
        REFLECTION
        REFLECTION_FIELD(ID)
        REFLECTION_FIELD(name)
        REFLECTION_FIELD(uri)
    private:
        Item()
        {
        }
    }
};
} // namespace indigo2

#endif