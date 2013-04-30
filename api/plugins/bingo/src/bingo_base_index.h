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
   class Matcher;
   class MatcherQueryData;

   class Index
   {
   public:
      virtual Matcher* createMatcher (const char *type, MatcherQueryData *query_data) = 0;

      virtual void create (const char *location, const MoleculeFingerprintParameters &fp_params, const char *options) = 0;

      virtual void load (const char *location) = 0;

      virtual int add (IndexObject &obj, int obj_id) = 0;

      virtual void remove (int id) = 0;
   
      typedef enum {MOLECULE, REACTION} IndexType;

      virtual const char * getIdPropertyName () = 0;

      virtual IndexType getType () const = 0;

      virtual ~Index () {};
   };
   
   class BaseIndex : public Index
   {
   public:
      virtual void create (const char *location, const MoleculeFingerprintParameters &fp_params, const char *options);

      virtual void load (const char *location);
      
      virtual int add (IndexObject &obj, int obj_id);

      virtual void remove (int id);

      const MoleculeFingerprintParameters & getFingerprintParams () const;

      const TranspFpStorage & getSubStorage () const;

      const RowFpStorage & getSimStorage () const;

      const Array<int> & getIdMapping () const;

      const Array<int> & getBackIdMapping () const;

      CfStorage & getCfStorage ();

      int getObjectsCount () const;

      virtual const char * getIdPropertyName ();

      virtual IndexType getType () const;

      static const char * determineType (const char *location);

      virtual ~BaseIndex ();

   protected:
      BaseIndex (IndexType type);
      IndexType _type;

   private:
      struct _ObjectIndexData 
      {
         Array<byte> sub_fp;
         Array<byte> sim_fp;
         Array<char> cf_str;
      };

      Array<int> _id_mapping;
      Array<int> _back_id_mapping;
      _ObjectIndexData _object_index_data;
      TranspFpStorage _sub_fp_storage;
      RowFpStorage _sim_fp_storage;
      MoleculeFingerprintParameters _fp_params;
      CfStorage _cf_storage;
      AutoPtr<StorageManager> _storage_manager;
      Properties _properties;
      std::string _location;
      std::ofstream _mapping_outfile;

      int _object_count;

      int _first_free_id;

      void _parseOptions (const char *options);

      void _saveProperties (const MoleculeFingerprintParameters &fp_params, int sub_block_size, int sim_block_size);

      bool _prepareIndexData (IndexObject &obj);

      void _insertIndexData();

      void _mappingLoad (const char * mapping_path);

      void _mappingAssign (int obj_id, int base_id);

      void _mappingAdd (int obj_id, int base_id);

      void _mappingRemove (int obj_id);
   };
};

#endif // __bingo_base_index__
