#ifndef __bingo_base_index__
#define __bingo_base_index__

#include "molecule/molecule_fingerprint.h"
#include "bingo_object.h"
#include "bingo_storage_manager.h"
#include "bingo_fp_storage.h"
#include "bingo_cf_storage.h"
#include "bingo_properties.h"

using namespace indigo;

namespace bingo
{
   typedef enum {IND_NO_TYPE = -1, IND_MOL, IND_RXN} IndexType;

   class Matcher;
   class MatcherQueryData;

   class Index
   {
   public:
      // Usage:
      //    createMatcher("sub", SubstructureMatcherQuery("C*N"));
      //    createMatcher("sub-fast", SubstructureMatcherQuery("C*N"));
      
      virtual Matcher* createMatcher (const char *type, const MatcherQueryData *query_data) = 0;

      virtual void create( const char *location, const MoleculeFingerprintParameters &fp_params ) = 0;

      virtual void load( const char *location ) = 0;

      virtual int add( /* const */ IndexObject &obj ) = 0;
      virtual void remove( int id ) = 0;
   
      virtual IndexType getType() = 0;
   };
   
   class BaseIndex : public Index
   {
   public:
      BaseIndex();

      // Usage:
      //    createMatcher("sub", SubstructureMatcherQuery("C*N"));
      //    createMatcher("sub-fast", SubstructureMatcherQuery("C*N"));

      virtual void create( const char *location, const MoleculeFingerprintParameters &fp_params );

      virtual void load( const char *location );
      
      virtual int add( /* const */ IndexObject &obj );

      virtual void remove( int id );

      const MoleculeFingerprintParameters & getFingerprintParams() const;

      const TranspFpStorage & getSubStorage() const;

      const RowFpStorage & getSimStorage() const;

      /*const */CfStorage & getCfStorage() /*const*/;

      int getObjectsCount() const;

      virtual IndexType getType();

      virtual ~BaseIndex();

   protected:
      TranspFpStorage _sub_fp_storage;
      RowFpStorage _sim_fp_storage;
      MoleculeFingerprintParameters _fp_params;
      CfStorage _cf_storage;
      FileStorageManager _file_storage_manager;
      IndexType _type;
      Properties _properties;

      int _object_count;

      std::string _location;
      std::string _sub_filename;
      std::string _sub_info_filename;
      std::string _sim_filename;
      std::string _sim_info_filename;
      std::string _props_filename;
      std::string _cf_data_filename;
      std::string _cf_offset_filename;

      void _saveProperties( const MoleculeFingerprintParameters &fp_params, int sub_block_size, int sim_block_size );
   };
};

#endif // __bingo_base_index__
