#include "bingo_base_index.h"
#include "bingo_mmf.h"
#include "bingo_ptr.h"

#include <sstream>
#include <string>
#include <limits.h>

#include "base_cpp/profiling.h"
#include "base_cpp/output.h"
#include "base_c/os_dir.h"

using namespace bingo;

static const char *_cf_data_filename = "cf_data";
static const char *_cf_offset_filename = "cf_offset";
static const char *_id_mapping_filename = "id_mapping";
static const char *_reaction_type = "reaction";
static const char *_molecule_type = "molecule";
static const int _type_len = 9;
static const char *_mmf_file = "mmf_storage";
static const char *_version_prop = "version";
static const size_t _mmf_size = 536870912; // 500Mb
static const int _sim_mt_size = 50000;

BaseIndex::BaseIndex (IndexType type)
{
   _type = type;
   _read_only = false;
   _object_count = 0;
   _first_free_id = 0;
   _index_id = -1;
}

void BaseIndex::create (const char *location, const MoleculeFingerprintParameters &fp_params, const char *options, int index_id)
{
   // TODO: introduce global parameters table, local parameters table and constants
   MMFStorage::database_id = index_id;

   int sub_block_size = 8192;
   int sim_block_size = 8192;
   int cf_block_size = 1048576;

   osDirCreate(location);

   _location = location;
   
   std::string _cf_data_path = _location + _cf_data_filename;
   std::string _cf_offset_path = _location + _cf_offset_filename;
   std::string _mapping_path = _location + _id_mapping_filename;
   std::string _mmf_path = _location + _mmf_file;

   _fp_params = fp_params;

   std::map<std::string, std::string> option_map;

   _parseOptions(options, option_map);

   _read_only = _getAccessType(option_map);

   size_t mmf_size = _getMMfSize(option_map);

   if (_type == MOLECULE)
      _mmf_storage.create(_mmf_path.c_str(), mmf_size, _molecule_type, index_id);
   else if (_type == REACTION)
      _mmf_storage.create(_mmf_path.c_str(), mmf_size, _reaction_type, index_id);
   else
      throw Exception("incorrect index type");

   _header.allocate();

   _header->properties_offset = Properties::create(_properties);

   _saveProperties(fp_params, sub_block_size, sim_block_size, cf_block_size, option_map);

   _properties->add(_version_prop, BINGO_VERSION);

   unsigned long prop_mt_size =  _properties->getULongNoThrow("mt_size");
   int mt_size = (prop_mt_size != ULONG_MAX ? prop_mt_size : _sim_mt_size);

   _mappingCreate();

   _header->cf_offset = ByteBufferStorage::create(_cf_storage, cf_block_size);
   _header->sub_offset = TranspFpStorage::create(_sub_fp_storage, _fp_params.fingerprintSize(), sub_block_size);
   _header->sim_offset = FingerprintTable::create(_sim_fp_storage, _fp_params.fingerprintSizeSim(), mt_size);
   _header->exact_offset = ExactStorage::create(_exact_storage);
}

void BaseIndex::load (const char *location, const char *options, int index_id)
{
   MMFStorage::database_id = index_id;

   if (osDirExists(location) == OS_DIR_NOTFOUND)
      throw Exception("database directory missed");

   osDirCreate(location);

   _location = location;
   std::string _cf_data_path = _location + _cf_data_filename;
   std::string _cf_offset_path = _location + _cf_offset_filename;
   std::string _mapping_path = _location + _id_mapping_filename;
   std::string _mmf_path = _location + _mmf_file;

   std::map<std::string, std::string> option_map;

   _parseOptions(options, option_map);

   _read_only = _getAccessType(option_map);

   BingoPtr<char> h_ptr;

   _mmf_storage.load(_mmf_path.c_str(), h_ptr, index_id, _read_only);
   
   _header = BingoPtr<_Header>(MMFStorage::max_header_len + BingoAllocator::getAllocatorDataSize());

   Properties::load(_properties, _header->properties_offset);
   
   const char *ver = _properties->get(_version_prop);

   if (strcmp(ver, BINGO_VERSION) != 0)
      throw Exception("BaseIndex: load(): incorrect database version");

   const char *type_str = (_type == MOLECULE ? _molecule_type : _reaction_type);
   if (strcmp(_properties->get("base_type"), type_str) != 0)
      throw Exception("Loading databse: wrong type propety");
   
   _fp_params.ext = (_properties.ref().getULong("fp_ext") != 0);
   _fp_params.ord_qwords = _properties.ref().getULong("fp_ord");
   _fp_params.any_qwords = _properties.ref().getULong("fp_any");
   _fp_params.tau_qwords = _properties.ref().getULong("fp_tau");
   _fp_params.sim_qwords = _properties.ref().getULong("fp_sim");

   unsigned long cf_block_size = _properties->getULong("cf_block_size");

   _mappingLoad();

   FingerprintTable::load(_sim_fp_storage, _header.ptr()->sim_offset);
   ExactStorage::load(_exact_storage, _header.ptr()->exact_offset);
   TranspFpStorage::load(_sub_fp_storage, _header.ptr()->sub_offset);
   ByteBufferStorage::load(_cf_storage, _header.ptr()->cf_offset);
}

int BaseIndex::add (/* const */ IndexObject &obj, int obj_id, DatabaseLockData &lock_data)
{
   if (_read_only)
      throw Exception("insert fail: Read only index can't be changed");

   BingoArray<int> & back_id_mapping = _back_id_mapping_ptr.ref();

   if (obj_id != -1 && back_id_mapping.size() > obj_id && back_id_mapping[obj_id] != -1)
      throw Exception("insert fail: This id was already used");

   _ObjectIndexData _obj_data;
   {
      profTimerStart(t_in, "prepare_obj_data");      
      _prepareIndexData(obj, _obj_data);
   }
   
   WriteLock wlock(lock_data);
   {
      profTimerStart(t_in, "add_obj_data");   
      _insertIndexData(_obj_data);
   }

   {
      profTimerStart(t_in, "mapping_changing_1");      
      if (obj_id == -1)
      {
         int i;
         for (i = _first_free_id; i < back_id_mapping.size(); i++)
         {
            if (back_id_mapping[i] == -1)
            {
               _first_free_id = i;
               break;
            }
         }

         if (i == back_id_mapping.size())
            _first_free_id = back_id_mapping.size();

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

void BaseIndex::optimize ()
{
   if (_read_only)
      throw Exception("optimize fail: Read only index can't be changed");

   _sim_fp_storage.ptr()->optimize();
}

void BaseIndex::remove (int obj_id)
{
   if (_read_only)
      throw Exception("remove fail: Read only index can't be changed");

   BingoArray<int> & back_id_mapping = _back_id_mapping_ptr.ref();

   if (obj_id < 0 || obj_id >= back_id_mapping.size() || back_id_mapping[obj_id] == -1)
      throw Exception("There is no object with this id");

   _cf_storage->remove(back_id_mapping[obj_id]);
   _mappingRemove(obj_id);
}

const MoleculeFingerprintParameters & BaseIndex::getFingerprintParams () const
{
   return _fp_params;
}

TranspFpStorage & BaseIndex::getSubStorage ()
{
   return _sub_fp_storage.ref();
}

FingerprintTable & BaseIndex::getSimStorage ()
{
   return _sim_fp_storage.ref();
}

ExactStorage & BaseIndex::getExactStorage ()
{
   return _exact_storage.ref();
}

BingoArray<int> & BaseIndex::getIdMapping ()
{
   return _id_mapping_ptr.ref();
}

BingoArray<int> & BaseIndex::getBackIdMapping ()
{
   return _back_id_mapping_ptr.ref();
}

/*const */ByteBufferStorage & BaseIndex::getCfStorage ()// const
{
   return _cf_storage.ref();
}

int BaseIndex::getObjectsCount () const
{
   return _object_count;
}

const char * BaseIndex::getIdPropertyName ()
{
   return _properties.ref().getNoThrow("key");
}

const char * BaseIndex::getVersion ()
{
   return BINGO_VERSION;
}

Index::IndexType BaseIndex::getType () const
{
   return _type;
}

Index::IndexType BaseIndex::determineType (const char *location)
{
   std::string path(location);
   path += '/';
   path += _mmf_file;
   path += '0';
   std::ifstream fstream(path, std::ios::binary | std::ios::ate);
   
   char type[_type_len];
   fstream.read(type, _type_len);

   if (strcmp(type, _molecule_type) == 0)
      return MOLECULE;
   else if (strcmp(type, _reaction_type) == 0)
      return REACTION;
   else
      throw Exception("BingoIndex: determineType(): Incorrect database type");
}

BaseIndex::~BaseIndex()
{
   _mmf_storage.close();
}

void BaseIndex::_parseOptions (const char *options, std::map<std::string, std::string> &option_map)
{
   if (options == 0 || strlen(options) == 0)
      return;

   option_map.clear();

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

      option_map.insert(std::pair<std::string, std::string>(opt_name, opt_value));
   }
}

size_t BaseIndex::_getMMfSize (std::map<std::string, std::string> &option_map)
{
   size_t mmf_size = _mmf_size;

   if (option_map.find("mmf_size") != option_map.end())
   {
      unsigned long u_dec;
      std::istringstream isstr(option_map["mmf_size"]);
      isstr >> u_dec;

      if (u_dec != ULONG_MAX)
         mmf_size = u_dec * 1048576;
   }

   return mmf_size;
}

bool BaseIndex::_getAccessType (std::map<std::string, std::string> &option_map)
{
   if (option_map.find("read_only") != option_map.end())
   {
      if (option_map["read_only"].compare("true") == 0)
         return true;
   }

   return false;
}


void BaseIndex::_saveProperties (const MoleculeFingerprintParameters &fp_params, int sub_block_size, 
                                 int sim_block_size, int cf_block_size, 
                                 std::map<std::string, std::string> &option_map)
{
   _properties.ref().add("base_type", (_type == MOLECULE ? _molecule_type : _reaction_type));

   _properties.ref().add("fp_ext", _fp_params.ext);
   _properties.ref().add("fp_ord", _fp_params.ord_qwords);
   _properties.ref().add("fp_any", _fp_params.any_qwords);
   _properties.ref().add("fp_tau", _fp_params.tau_qwords);
   _properties.ref().add("fp_sim", _fp_params.sim_qwords);

   _properties.ref().add("cf_block_size", cf_block_size);

   std::map<std::string, std::string>::iterator it;
   for (it = option_map.begin(); it != option_map.end(); it++)
   {
      _properties->add(it->first.c_str(), it->second.c_str());
   }
}

bool BaseIndex::_prepareIndexData (IndexObject &obj, _ObjectIndexData &obj_data)
{
   {
      profTimerStart(t, "prepare_cf");
      if (!obj.buildCfString(obj_data.cf_str))
         return false;
   }

   {
      profTimerStart(t, "prepare_fp");
      if (!obj.buildFingerprint(_fp_params, &obj_data.sub_fp, &obj_data.sim_fp))
         return false;
   }

   if (!obj.buildHash(obj_data.hash))
      return false;

   return true;
}

void BaseIndex::_insertIndexData (_ObjectIndexData &obj_data)
{
   _sub_fp_storage.ptr()->add(obj_data.sub_fp.ptr());
   _sim_fp_storage.ptr()->add(obj_data.sim_fp.ptr(), _object_count);
   _cf_storage.ptr()->add((byte *)obj_data.cf_str.ptr(), obj_data.cf_str.size(), _object_count);
   _exact_storage.ptr()->add(obj_data.hash, _object_count);
}

void BaseIndex::_mappingLoad ()
{
   _id_mapping_ptr = BingoPtr< BingoArray<int> >(_header->mapping_offset);
   _back_id_mapping_ptr = BingoPtr< BingoArray<int> >(_header->back_mapping_offset);

   return;
}

void BaseIndex::_mappingCreate ()
{
   _id_mapping_ptr.allocate();
   new(_id_mapping_ptr.ptr()) BingoArray<int>();
   _header->mapping_offset = (size_t)_id_mapping_ptr;

   _back_id_mapping_ptr.allocate();
   new(_back_id_mapping_ptr.ptr()) BingoArray<int>();
   _header->back_mapping_offset = (size_t)_back_id_mapping_ptr;
}

void BaseIndex::_mappingAssign (int obj_id, int base_id)
{
   BingoArray<int> & id_mapping = _id_mapping_ptr.ref();
   BingoArray<int> & back_id_mapping = _back_id_mapping_ptr.ref();

   int old_size = id_mapping.size();
   int old_back_size = back_id_mapping.size();
   if (id_mapping.size() <= base_id)
      id_mapping.resize(base_id + 1);
   if (back_id_mapping.size() <= obj_id)
      back_id_mapping.resize(obj_id + 1);

   for (int i = old_size; i < id_mapping.size(); i++)
      id_mapping[i] = -1;

   for (int i = old_back_size; i < back_id_mapping.size(); i++)
      back_id_mapping[i] = -1;

   id_mapping[base_id] = obj_id;
   
   if (obj_id == -1)
      return;
   
   if (back_id_mapping[obj_id] != -1)
      throw Exception("insert fail: this id was already used");

   back_id_mapping[obj_id] = base_id;
}

void BaseIndex::_mappingAdd (int obj_id, int base_id)
{
   _mappingAssign(obj_id, base_id);
}

void BaseIndex::_mappingRemove (int obj_id)
{
   BingoArray<int> & id_mapping = _id_mapping_ptr.ref();
   BingoArray<int> & back_id_mapping = _back_id_mapping_ptr.ref();

   if (back_id_mapping[obj_id] != -1)
   {
      int new_id = -1;

      id_mapping[back_id_mapping[obj_id]] = -1;
      back_id_mapping[obj_id] = -1;
   }
}
