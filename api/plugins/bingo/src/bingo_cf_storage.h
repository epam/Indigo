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

      void create( const char *cf_filename, const char *offset_filename );

      void load( const char *cf_filename, const char *offset_filename );

      const char * get( int idx, int &len );

      void add( const char *data, int len, int idx );

      void remove( int idx );

   private:
      static const int _max_cf_len = 30;

      struct _Addr
      {
         int offset;
         short len;
      };
      
      int _cf_count;

      char _cur_cf_str[_max_cf_len];
      std::fstream _cf_file;
      std::fstream _offset_file;
      std::string _cf_filename;
      std::string _offset_filename;
   };
};

#endif /* __cmf_storage__ */
