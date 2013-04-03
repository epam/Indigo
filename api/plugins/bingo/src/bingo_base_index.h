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
      // Usage:
      //    createMatcher("sub", SubstructureMatcherQuery("C*N"));
      //    createMatcher("sub-fast", SubstructureMatcherQuery("C*N"));
      
      virtual Matcher* createMatcher (const char *type, MatcherQueryData *query_data) = 0;

      virtual void create (const char *location, const MoleculeFingerprintParameters &fp_params) = 0;

      virtual void load (const char *location) = 0;

      virtual int add (/* const */ IndexObject &obj) = 0;
      virtual void remove (int id) = 0;
   
      typedef enum {MOLECULE, REACTION} IndexType;

      virtual IndexType getType () = 0;

      virtual ~Index () {};
   };
   
   class BaseIndex : public Index
   {
   public:
      // Usage:
      //    createMatcher("sub", SubstructureMatcherQuery("C*N"));
      //    createMatcher("sub-fast", SubstructureMatcherQuery("C*N"));

      virtual void create (const char *location, const MoleculeFingerprintParameters &fp_params);

      virtual void load (const char *location);
      
      virtual int add (/* const */ IndexObject &obj);

      virtual void remove (int id);

      const MoleculeFingerprintParameters & getFingerprintParams () const;

      const TranspFpStorage & getSubStorage () const;

      const RowFpStorage & getSimStorage () const;

      /*const */CfStorage & getCfStorage () /*const*/;

      int getObjectsCount () const;

      virtual IndexType getType ();

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

      _ObjectIndexData _object_index_data;
      TranspFpStorage _sub_fp_storage;
      RowFpStorage _sim_fp_storage;
      MoleculeFingerprintParameters _fp_params;
      CfStorage _cf_storage;
      AutoPtr<FileStorageManager> _file_storage_manager;
      Properties _properties;
      std::string _location; // TODO: move to StorageManager --DONE

      int _object_count;

      void _saveProperties (const MoleculeFingerprintParameters &fp_params, int sub_block_size, int sim_block_size);

      bool _prepareIndexData (IndexObject &obj);

      void _insertIndexData();
   };
};

#endif // __bingo_base_index__
