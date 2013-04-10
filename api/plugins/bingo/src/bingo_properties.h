#ifndef __bingo_parameters__
#define __bingo_parameters__

#include <iostream>
#include <fstream>
#include <string>
#include <map>

namespace bingo
{
   class Properties
   {
   public:
      Properties ();

      void create (const char *filename);

      void load (const char *filename);

      void add (const char *prop_name, const char *value);

      void add (const char *prop_name, unsigned long value);

      const char * get (const char *prop_name);

      unsigned long getULong (const char *prop_name);

   private:
      typedef std::pair<const std::string, std::string> _PropertyPair;

      void _rewritePropFile ();

      static void _parseProperty (const std::string &line, std::string &prop_out, std::string &value_out);

      std::string _filename;
      std::map<const std::string, std::string> _props;
   };
};

#endif /* __bingo_parameters__ */