#ifndef __cmf_storage__
#define __cmf_storage__

#include "base_cpp\array.h"
#include "base_cpp\tlscont.h"
#include "base_cpp\tlscont.h"

#include <iostream>
#include <fstream>

using namespace indigo;

namespace bingo
{
   class CfStorage
   {
   public:
      CfStorage( void );

      void create (const char *cf_filename, const char *offset_filename);

      void load (const char *cf_filename, const char *offset_filename);

      const char * get (int idx, int &len);

      void add (const char *data, int len, int idx);

      void remove (int idx);

   private:
      static const int _max_cf_len = 1024;

      struct _Addr
      {
         int offset;
         int len;
      };

      int _cf_count;

      char _cur_cf_str[_max_cf_len];
      std::ifstream _cf_infile;
      std::ifstream _offset_infile;
      std::ofstream _cf_outfile;
      std::ofstream _offset_outfile;
      std::string _cf_filename;
      std::string _offset_filename;
   };
};

#endif /* __cmf_storage__ */
