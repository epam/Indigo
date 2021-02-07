#ifndef __reflection__
#define __reflection__

#include <string>
#include <vector>

#define REFLECTION                                                                                                                                             \
    inline virtual void reflection(Serializer& fieldsScanner, const std::vector<const std::string> fields = {}, bool exclude = false)                       \
    {                                                                                                                                                          \
        if (0)                                                                                                                                                 \
        {                                                                                                                                                      \
        }
#define REFLECTION_FIELD(NAME)                                                                                                                                 \
    else if (fields.empty() || (!exclude && std::find(fields.begin(), fields.end(), ##NAME) != fields.end()) ||                                                \
             (exclude && std::find(fields.begin(), fields.end(), ##NAME) == fields.end()))                                                                     \
    {                                                                                                                                                          \
        fieldsScanner.process(                                                                                                                                 \
            ##NAME, [this]() { return this->NAME() }, [this](auto value) { this->NAME(value) });                                                               \
    }
#define REFLECTION_END(SUPERCLASS)                                                                                                                             \
    SUPERCLASS::reflection(fields_scanner, fields, exclude);                                                                                                   \
    }

#endif
