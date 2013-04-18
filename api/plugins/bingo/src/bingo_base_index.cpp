#include "bingo_base_index.h"
#include "bingo_storage.h"

#include <sstream>
#include <string>

#include "base_cpp/profiling.h"
#include "base_cpp/output.h"

using namespace bingo;

static const char *_sub_filename = "sub_fp.fp";
static const char *_sub_info_filename = "sub_fp_info.fp";
static const char *_sim_filename = "sim_fp.fp";
static const char *_sim_info_filename = "sim_fp_info.fp";
static const char *_props_filename = "properties";
static const char *_cf_data_filename = "cf_data";
static const char *_cf_offset_filename = "cf_offset";

BaseIndex::BaseIndex (IndexType type)
{
   _type = type;

   _object_count = 0;
}

// Usage:
//    createMatcher("sub", SubstructureMatcherQuery("C*N"));
//    createMatcher("sub-fast", SubstructureMatcherQuery("C*N"));

void BaseIndex::create (const char *location, const MoleculeFingerprintParameters &fp_params)
{
   // TODO: introduce global parameters table, local parameters table and constants

   // TODO: create storage manager in a specified location --DONE

   int sub_block_size = 8192;
   int sim_block_size = 8192;

   _location = location;
   std::string sub_info_path = _location + _sub_info_filename;
   std::string sim_info_path = _location + _sim_info_filename;
   std::string props_path = _location + _props_filename;
   std::string _cf_data_path = _location + _cf_data_filename;
   std::string _cf_offset_path = _location + _cf_offset_filename;

   _fp_params = fp_params;
   
   _properties.create(props_path.c_str());

   _saveProperties(fp_params, sub_block_size, sim_block_size);

   _storage_manager.reset(new RamStorageManager(location));

   AutoPtr<Storage> sub_stor = _storage_manager->create(_sub_filename, sub_block_size);
   AutoPtr<Storage> sim_stor = _storage_manager->create(_sim_filename, sim_block_size);

   _sub_fp_storage.create(_fp_params.fingerprintSize(), sub_stor.release(), sub_info_path.c_str());
   _sim_fp_storage.create(_fp_params.fingerprintSizeSim(), sim_stor.release(), sim_info_path.c_str());
   
   _cf_storage.create(_cf_data_path.c_str(), _cf_offset_path.c_str());
}

void BaseIndex::load (const char *location)
{
   _location = location;
   std::string sub_info_path = _location + _sub_info_filename;
   std::string sim_info_path = _location + _sim_info_filename;
   std::string props_path = _location + _props_filename;
   std::string _cf_data_path = _location + _cf_data_filename;
   std::string _cf_offset_path = _location + _cf_offset_filename;

   _properties.load(props_path.c_str());

   _fp_params.ext = (_properties.getULong("fp_ext") != 0);
   _fp_params.ord_qwords = _properties.getULong("fp_ord");
   _fp_params.any_qwords = _properties.getULong("fp_any");
   _fp_params.tau_qwords = _properties.getULong("fp_tau");
   _fp_params.sim_qwords = _properties.getULong("fp_sim");

   int sub_block_size = _properties.getULong("sub_block_size");
   int sim_block_size = _properties.getULong("sim_block_size");

   std::istringstream isstr(std::string(_properties.get("fp_params")));

   isstr >> _fp_params.ext >>
           _fp_params.ord_qwords >>
           _fp_params.any_qwords >>
           _fp_params.tau_qwords >>
           _fp_params.sim_qwords;

   _storage_manager.reset(new RamStorageManager(location));

   AutoPtr<Storage> sub_stor = _storage_manager->load(_sub_filename);
   AutoPtr<Storage> sim_stor = _storage_manager->load(_sim_filename);

   _sub_fp_storage.load(_fp_params.fingerprintSize(), sub_stor.release(), sub_info_path.c_str());
   _sim_fp_storage.load(_fp_params.fingerprintSizeSim(), sim_stor.release(), sim_info_path.c_str());

   _cf_storage.load(_cf_data_path.c_str(), _cf_offset_path.c_str());
}

int BaseIndex::add (/* const */ IndexObject &obj)
{
   // TODO: Split prepare and add into index because of potential 
   //    MoleculeIndex features: molecule mass, molecular formula, etc.
   // Prepare + atomic Add --DONE
   {
      profTimerStart(t_in, "prepare_obj_data");      
      _prepareIndexData(obj);
   }

   {
      profTimerStart(t_in, "add_obj_data");      
      _insertIndexData();
   }
   
   return _object_count++;
}

void BaseIndex::remove (int id)
{
   throw Exception("Not implemented yet...");
}

const MoleculeFingerprintParameters & BaseIndex::getFingerprintParams () const
{
   return _fp_params;
}

const TranspFpStorage & BaseIndex::getSubStorage () const
{
   return _sub_fp_storage;
}

const RowFpStorage & BaseIndex::getSimStorage () const
{
   return _sim_fp_storage;
}

/*const */CfStorage & BaseIndex::getCfStorage ()// const
{
   return _cf_storage;
}

int BaseIndex::getObjectsCount () const
{
   return _object_count;
}

Index::IndexType BaseIndex::getType ()
{
   return _type;
}

BaseIndex::~BaseIndex()
{
}

void BaseIndex::_saveProperties (const MoleculeFingerprintParameters &fp_params, int sub_block_size, int sim_block_size)
{
   // TODO: separate fp parameters --DONE
   _properties.add("fp_ext", _fp_params.ext);
   _properties.add("fp_ord", _fp_params.ord_qwords);
   _properties.add("fp_any", _fp_params.any_qwords);
   _properties.add("fp_tau", _fp_params.tau_qwords);
   _properties.add("fp_sim", _fp_params.sim_qwords);

   // TODO: Properties.add(string, int) --DONE
   // Properties.getInt(string), etc. --DONE
   _properties.add("sub_block_size", sub_block_size);

   _properties.add("sim_block_size", sim_block_size);
}

bool BaseIndex::_prepareIndexData (IndexObject &obj)
{
   if (!obj.buildCfString(_object_index_data.cf_str))
      return false;

   if (!obj.buildFingerprint(_fp_params, &_object_index_data.sub_fp, &_object_index_data.sim_fp))
      return false;

   return true;
}

void BaseIndex::_insertIndexData ()
{
   _sub_fp_storage.add(_object_index_data.sub_fp.ptr());
   _sim_fp_storage.add(_object_index_data.sim_fp.ptr());
   _cf_storage.add(_object_index_data.cf_str.ptr(), _object_index_data.cf_str.size(), _object_count);
}
