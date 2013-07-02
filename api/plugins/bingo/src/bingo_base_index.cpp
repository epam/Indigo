#include "bingo_base_index.h"
#include "bingo_storage.h"

#include <sstream>
#include <string>

#include "base_cpp/profiling.h"
#include "base_cpp/output.h"
#include "base_c/os_dir.h"

using namespace bingo;

static const char *_sub_filename = "sub_fp.fp";
static const char *_sub_info_filename = "sub_fp_info.fp";
static const char *_sim_filename = "sim_fp.fp";
static const char *_sim_info_filename = "sim_fp_info.fp";
static const char *_props_filename = "properties";
static const char *_cf_data_filename = "cf_data";
static const char *_cf_offset_filename = "cf_offset";
static const char *_id_mapping_filename = "id_mapping";
static const char *_reaction_type = "reaction";
static const char *_molecule_type = "molecule";
static const char *_ram_storage_type = "ram";
static const char *_file_storage_type = "file";

BaseIndex::BaseIndex (IndexType type)
{
   _type = type;

   _object_count = 0;
   _first_free_id = 0;
}

void BaseIndex::create (const char *location, const MoleculeFingerprintParameters &fp_params, const char *options)
{
   // TODO: introduce global parameters table, local parameters table and constants

   int sub_block_size = 8192;
   int sim_block_size = 8192;
   int cf_block_size = 1048576;

   osDirCreate(location);

   _location = location;
   
   std::string sub_info_path = _location + _sub_info_filename;
   std::string sim_info_path = _location + _sim_info_filename;
   std::string props_path = _location + _props_filename;
   std::string _cf_data_path = _location + _cf_data_filename;
   std::string _cf_offset_path = _location + _cf_offset_filename;
   std::string _mapping_path = _location + _id_mapping_filename;

   _fp_params = fp_params;
   
   _properties.create(props_path.c_str());

   _parseOptions(options);

   _saveProperties(fp_params, sub_block_size, sim_block_size, cf_block_size);

   if ((_properties.get("storage") == 0) || strcmp(_properties.get("storage"), _ram_storage_type) == 0)
      _storage_manager.reset(new RamStorageManager(location, true));
   else if (strcmp(_properties.get("storage"), _file_storage_type) == 0)
      _storage_manager.reset(new FileStorageManager(location, true));
   else
      throw Exception("Unknown storage type");

   _mapping_outfile.open(_mapping_path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

   AutoPtr<Storage> sub_stor(_storage_manager->create(_sub_filename, sub_block_size));
   AutoPtr<Storage> sim_stor(_storage_manager->create(_sim_filename, sim_block_size));
   _cf_storage.reset(new ByteBufferStorage(cf_block_size));

   _sub_fp_storage.create(_fp_params.fingerprintSize(), sub_stor.release(), sub_info_path.c_str());
   _sim_fp_storage.create(_fp_params.fingerprintSizeSim(), sim_stor.release(), sim_info_path.c_str());
   
   _cf_storage->create(_cf_data_path.c_str(), _cf_offset_path.c_str());
}

void BaseIndex::load (const char *location, const char *options)
{
   if (osDirExists(location) == OS_DIR_NOTFOUND)
      throw Exception("database directory missed");

   osDirCreate(location);

   _location = location;
   std::string sub_info_path = _location + _sub_info_filename;
   std::string sim_info_path = _location + _sim_info_filename;
   std::string props_path = _location + _props_filename;
   std::string _cf_data_path = _location + _cf_data_filename;
   std::string _cf_offset_path = _location + _cf_offset_filename;
   std::string _mapping_path = _location + _id_mapping_filename;

   _properties.load(props_path.c_str());

   _parseOptions(options);

   const char *type_str = (_type == MOLECULE ? _molecule_type : _reaction_type);
   if (strcmp(_properties.get("base_type"), type_str) != 0)
      throw Exception("Loading databse: wrong type propety");

   _fp_params.ext = (_properties.getULong("fp_ext") != 0);
   _fp_params.ord_qwords = _properties.getULong("fp_ord");
   _fp_params.any_qwords = _properties.getULong("fp_any");
   _fp_params.tau_qwords = _properties.getULong("fp_tau");
   _fp_params.sim_qwords = _properties.getULong("fp_sim");

   unsigned long cf_block_size = _properties.getULong("cf_block_size");

   _mappingLoad(_mapping_path.c_str());

   if ((_properties.get("storage") == 0) || (strcmp(_properties.get("storage"), _ram_storage_type) == 0))
      _storage_manager.reset(new RamStorageManager(location, false));
   else if (strcmp(_properties.get("storage"), _file_storage_type) == 0)
      _storage_manager.reset(new FileStorageManager(location, false));
   else
      throw Exception("Unknown storage type");

    _mapping_outfile.open(_mapping_path.c_str(), std::ios::in | std::ios::out | std::ios::binary);

   AutoPtr<Storage> sub_stor(_storage_manager->load(_sub_filename));
   AutoPtr<Storage> sim_stor(_storage_manager->load(_sim_filename));
   _cf_storage.reset(new ByteBufferStorage(cf_block_size));

   _sub_fp_storage.load(_fp_params.fingerprintSize(), sub_stor.release(), sub_info_path.c_str());
   _sim_fp_storage.load(_fp_params.fingerprintSizeSim(), sim_stor.release(), sim_info_path.c_str());

   _cf_storage->load(_cf_data_path.c_str(), _cf_offset_path.c_str());
}

int BaseIndex::add (/* const */ IndexObject &obj, int obj_id)
{
   {
      profTimerStart(t_in, "prepare_obj_data");      
      _prepareIndexData(obj);
   }

   {
      profTimerStart(t_in, "add_obj_data");      
      _insertIndexData();
   }

   {
      profTimerStart(t_in, "mapping_changing_1");      
      if (obj_id == -1)
      {
         int i;
         for (i = _first_free_id; i < _back_id_mapping.size(); i++)
         {
            if (_back_id_mapping[i] == -1)
            {
               _first_free_id = i;
               break;
            }
         }

         if (i == _back_id_mapping.size())
            _first_free_id = _back_id_mapping.size();

         obj_id = _first_free_id;
      }
   }

   int base_id = _object_count;
   _object_count++;
   {
      profTimerStart(t_in, "mapping_changing_2");    
      _mappingAdd(obj_id, base_id);
   }
   
   return obj_id;
}

void BaseIndex::remove (int obj_id)
{
   if (obj_id < 0 || obj_id >= _back_id_mapping.size() || _back_id_mapping[obj_id] == -1)
      throw Exception("There is no object with this id");

   _cf_storage->remove(_back_id_mapping[obj_id]);
   _mappingRemove(obj_id);
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

const Array<int> & BaseIndex::getIdMapping () const
{
   return _id_mapping;
}

const Array<int> & BaseIndex::getBackIdMapping () const
{
   return _back_id_mapping;
}

/*const */FlatStorage & BaseIndex::getCfStorage ()// const
{
   return _cf_storage.ref();
}

int BaseIndex::getObjectsCount () const
{
   return _object_count;
}

const char * BaseIndex::getIdPropertyName ()
{
   return _properties.get("key");
}

Index::IndexType BaseIndex::getType () const
{
   return _type;
}

const char * BaseIndex::determineType (const char *location)
{
   Properties props;
   std::string path(location);
   path += _props_filename;

   props.load(path.c_str());
   return props.get("base_type");
}

BaseIndex::~BaseIndex()
{

}

void BaseIndex::_parseOptions (const char *options)
{
   if (options == 0 || strlen(options) == 0)
      return;

   std::stringstream options_stream;
   options_stream << options;

   std::string line;
   while (options_stream.good())
   {
      std::getline(options_stream, line, ';');

      if (line.size() == 0)
         continue;

      std::string opt_name, opt_value;
      int sep = (int)line.find_first_of(':');

      opt_name.assign(line.substr(0, sep));
      opt_value.assign(line.substr(sep + 1, std::string::npos));

      _properties.add(opt_name.c_str(), opt_value.c_str());
   }
}

void BaseIndex::_saveProperties (const MoleculeFingerprintParameters &fp_params, int sub_block_size, 
                                 int sim_block_size, int cf_block_size)
{
   _properties.add("base_type", (_type == MOLECULE ? _molecule_type : _reaction_type));

   _properties.add("fp_ext", _fp_params.ext);
   _properties.add("fp_ord", _fp_params.ord_qwords);
   _properties.add("fp_any", _fp_params.any_qwords);
   _properties.add("fp_tau", _fp_params.tau_qwords);
   _properties.add("fp_sim", _fp_params.sim_qwords);

   _properties.add("cf_block_size", cf_block_size);
}

bool BaseIndex::_prepareIndexData (IndexObject &obj)
{
   {
      profTimerStart(t, "prepare_cf");
      if (!obj.buildCfString(_object_index_data.cf_str))
         return false;
   }

   {
      profTimerStart(t, "prepare_fp");
      if (!obj.buildFingerprint(_fp_params, &_object_index_data.sub_fp, &_object_index_data.sim_fp))
         return false;
   }

   return true;
}

void BaseIndex::_insertIndexData ()
{
   _sub_fp_storage.add(_object_index_data.sub_fp.ptr());
   _sim_fp_storage.add(_object_index_data.sim_fp.ptr());
   _cf_storage->add((byte *)_object_index_data.cf_str.ptr(), _object_index_data.cf_str.size(), _object_count);
}

void BaseIndex::_mappingLoad (const char * mapping_path)
{
   std::ifstream mapping_file(mapping_path, std::ios::in | std::ios::binary);

   if (!mapping_file.is_open())
      throw Exception("mapping file missed");

   int obj_id = -1;
   
   mapping_file.seekg(0, std::ios::end);
   size_t file_len = mapping_file.tellg();
   size_t mapping_size = (file_len - sizeof(_object_count)) / sizeof(obj_id);

   mapping_file.seekg(std::ios::beg);
   mapping_file.read((char *)&_object_count, sizeof(_object_count));
   

   std::vector<int> mapping_buf;
   mapping_buf.resize(mapping_size);

   mapping_file.read((char *)&mapping_buf[0], sizeof(obj_id) * mapping_size);

   for (int base_id = 0; base_id < mapping_size; base_id++)
   {
      obj_id = mapping_buf[base_id];
      _mappingAssign(obj_id, base_id);
   }

   return;
}

void BaseIndex::_mappingAssign (int obj_id, int base_id)
{
   if (_id_mapping.size() <= base_id)
      _id_mapping.expandFill(base_id + 1, -1);
   if (_back_id_mapping.size() <= obj_id)
      _back_id_mapping.expandFill(obj_id + 1, -1);

   _id_mapping[base_id] = obj_id;
   
   if (obj_id == -1)
      return;
   
   if (_back_id_mapping[obj_id] != -1)
      throw Exception("insert fail: this id was already used");

   _back_id_mapping[obj_id] = base_id;
}

void BaseIndex::_mappingAdd (int obj_id, int base_id)
{
   _mappingAssign(obj_id, base_id);

   _mapping_outfile.seekp(std::ios::beg);
   _mapping_outfile.write((char *)&_object_count, sizeof(obj_id));
   _mapping_outfile.seekp((size_t)sizeof(_object_count) + base_id * sizeof(obj_id));
   _mapping_outfile.write((char *)&obj_id, sizeof(obj_id));
   _mapping_outfile.flush();
}

void BaseIndex::_mappingRemove (int obj_id)
{
   if (_back_id_mapping[obj_id] != -1)
   {
      int new_id = -1;
      _mapping_outfile.seekp((size_t)sizeof(_object_count) + _back_id_mapping[obj_id] * sizeof(obj_id));
      _mapping_outfile.write((char *)&new_id, sizeof(new_id));
      _mapping_outfile.flush();

      _id_mapping[_back_id_mapping[obj_id]] = -1;
      _back_id_mapping[obj_id] = -1;
   }
}
