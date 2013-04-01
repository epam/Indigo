#include "bingo_base_index.h"
#include "bingo_storage.h"

#include <sstream>
#include <string>

using namespace bingo;

static const char *_sub_filename = "sub_fp.fp";
static const char *_sub_info_filename = "sub_fp_info.fp";
static const char *_sim_filename = "sim_fp.fp";
static const char *_sim_info_filename = "sim_fp_info.fp";
static const char *_props_filename = "properties";
static const char *_cf_data_filename = "cf_data";
static const char *_cf_offset_filename = "cf_offset";

BaseIndex::BaseIndex(IndexType type)
{
   _type = type;

   _object_count = 0;
}

// Usage:
//    createMatcher("sub", SubstructureMatcherQuery("C*N"));
//    createMatcher("sub-fast", SubstructureMatcherQuery("C*N"));

void BaseIndex::create( const char *location, const MoleculeFingerprintParameters &fp_params )
{
   // TODO: introduce global parameters table, local parameters table and constants

   // TODO: create storage manager in a specified location

   int sub_block_size = 128;
   int sim_block_size = 128;

   _location = location;
   std::string sub_path = _location + _sub_filename;
   std::string sub_info_path = _location + _sub_info_filename;
   std::string sim_path = _location + _sim_filename;
   std::string sim_info_path = _location + _sim_info_filename;
   std::string props_path = _location + _props_filename;
   std::string _cf_data_path = _location + _cf_data_filename;
   std::string _cf_offset_path = _location + _cf_offset_filename;

   _fp_params = fp_params;
   
   _properties.create(props_path.c_str());

   _saveProperties(fp_params, sub_block_size, sim_block_size);

   AutoPtr<Storage> sub_stor = _file_storage_manager.create(sub_path.c_str(), sub_block_size);
   AutoPtr<Storage> sim_stor = _file_storage_manager.create(sim_path.c_str(), sim_block_size);

   _sub_fp_storage.create(_fp_params.fingerprintSize(), sub_stor.release(), sub_info_path.c_str());
   _sim_fp_storage.create(_fp_params.fingerprintSizeSim(), sim_stor.release(), sim_info_path.c_str());
   
   _cf_storage.create(_cf_data_path.c_str(), _cf_offset_path.c_str());
}

void BaseIndex::load( const char *location )
{
   _location = location;
   std::string sub_path = _location + _sub_filename;
   std::string sub_info_path = _location + _sub_info_filename;
   std::string sim_path = _location + _sim_filename;
   std::string sim_info_path = _location + _sim_info_filename;
   std::string props_path = _location + _props_filename;
   std::string _cf_data_path = _location + _cf_data_filename;
   std::string _cf_offset_path = _location + _cf_offset_filename;

   _properties.load(props_path.c_str());

   int sub_block_size = atoi(_properties.get("sub_block_size"));
   int sim_block_size = atoi(_properties.get("sim_block_size"));

   std::istringstream isstr(std::string(_properties.get("fp_params")));

   isstr >> _fp_params.ext >>
           _fp_params.ord_qwords >>
           _fp_params.any_qwords >>
           _fp_params.tau_qwords >>
           _fp_params.sim_qwords;

   FileStorage *sub_stor = _file_storage_manager.load(sub_path.c_str(), sub_block_size);
   FileStorage *sim_stor = _file_storage_manager.load(sim_path.c_str(), sim_block_size);

   _sub_fp_storage.load(_fp_params.fingerprintSize(), sub_stor, sub_info_path.c_str());
   _sim_fp_storage.load(_fp_params.fingerprintSizeSim(), sim_stor, sim_info_path.c_str());

   _cf_storage.load(_cf_data_path.c_str(), _cf_offset_path.c_str());
}

int BaseIndex::add( /* const */ IndexObject &obj )
{
   // TODO: Split prepare and add into index because of potential 
   //    MoleculeIndex features: molecule mass, molecular formula, etc.
   // Prepare + atomic Add
   struct ObjectIndexData 
   {
      Array<byte> sub_fp;
      Array<byte> sim_fp;
      Array<char> cf_str;
   };
   QS_DEF(ObjectIndexData, data);

   obj.buildCfString(data.cf_str);
   obj.buildFingerprint(_fp_params, &data.sub_fp, &data.sim_fp);

   _sub_fp_storage.add(data.sub_fp.ptr());
   _sim_fp_storage.add(data.sim_fp.ptr());
   _cf_storage.add(data.cf_str.ptr(), data.cf_str.size(), _object_count);
   
   return _object_count++;
}

void BaseIndex::remove( int id )
{
   throw Exception("Not implemented yet...");
}

const MoleculeFingerprintParameters & BaseIndex::getFingerprintParams() const
{
   return _fp_params;
}

const TranspFpStorage & BaseIndex::getSubStorage() const
{
   return _sub_fp_storage;
}

const RowFpStorage & BaseIndex::getSimStorage() const
{
   return _sim_fp_storage;
}

/*const */CfStorage & BaseIndex::getCfStorage()// const
{
   return _cf_storage;
}

int BaseIndex::getObjectsCount() const
{
   return _object_count;
}

Index::IndexType BaseIndex::getType()
{
   return _type;
}

BaseIndex::~BaseIndex()
{
}

void BaseIndex::_saveProperties( const MoleculeFingerprintParameters &fp_params, int sub_block_size, int sim_block_size )
{
   // TODO: separate fp parameters
   std::stringstream sstr;
   sstr << _fp_params.ext << ' ' <<
           _fp_params.ord_qwords << ' ' <<
           _fp_params.any_qwords << ' ' <<
           _fp_params.tau_qwords << ' ' <<
           _fp_params.sim_qwords;

   _properties.add("fp_params", sstr.str().c_str());

   // TODO: Properties.add(string, int)
   // Properties.getInt(string), etc.
   sstr.str(std::string());
   sstr << sub_block_size;
   _properties.add("sub_block_size", sstr.str().c_str());

   sstr.str(std::string());
   sstr << sim_block_size;
   _properties.add("sim_block_size", sstr.str().c_str());
}
