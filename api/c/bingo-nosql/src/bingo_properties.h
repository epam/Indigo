#ifndef __bingo_parameters__
#define __bingo_parameters__

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "bingo_ptr.h"

namespace bingo
{
    class Properties
    {
    public:
        Properties();

        static BingoAddr create(BingoPtr<Properties>& ptr);

        static void load(BingoPtr<Properties>& ptr, BingoAddr offset);

        static void parseOptions(const char* options, std::map<std::string, std::string>& option_map, std::vector<std::string>* allowed_props = 0);

        void add(const char* prop_name, const char* value);

        void add(const char* prop_name, unsigned long value);

        const char* get(const char* prop_name);

        const char* getNoThrow(const char* prop_name) const;

        unsigned long getULong(const char* prop_name);

        unsigned long getULongNoThrow(const char* prop_name);

    private:
        struct _PropertyPair
        {
            BingoPtr<char> name;
            BingoPtr<char> value;
        };

        void _rewritePropFile();

        static void _parseProperty(const std::string& line, std::string& prop_out, std::string& value_out);

        static const int max_prop_len = 1024;
        BingoArray<_PropertyPair> _props;
    };
}; // namespace bingo

#endif /* __bingo_parameters__ */