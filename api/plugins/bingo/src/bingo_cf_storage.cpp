#include "bingo_cf_storage.h"

using namespace bingo;

CfStorage::CfStorage (void)
{
   _cf_count = 0;
}

void CfStorage::create (const char *cf_filename, const char *offset_filename)
{
   _cf_filename = cf_filename;
   _offset_filename = offset_filename;

   _cf_file.open(cf_filename, std::ios::out);
   _offset_file.open(offset_filename, std::ios::out);
}

void CfStorage::load (const char *cf_filename, const char *offset_filename)
{
   _cf_filename = cf_filename;
   _offset_filename = offset_filename;

   _cf_file.open(cf_filename, std::ios::out | std::ios::app | std::ios::binary);
   _offset_file.open(offset_filename, std::ios::out | std::ios::app | std::ios::binary);
}

const char * CfStorage::get (int idx, int &len)
{
   _cf_file.close();
   _offset_file.close();
   _cf_file.open(_cf_filename, std::ios::in | std::ios::app | std::ios::binary);
   _offset_file.open(_offset_filename, std::ios::in | std::ios::app | std::ios::binary);

   _Addr addr;

   _offset_file.seekg(idx * sizeof(addr));
   _offset_file.read((char *)(&addr), sizeof(addr));

   _cf_file.seekg(addr.offset);
   _cf_file.read(_cur_cf_str, addr.len);

   len = addr.len;

   return _cur_cf_str; 
}

void CfStorage::add (const char *data, int len, int idx)
{
   _cf_file.close();
   _offset_file.close();
   _cf_file.open(_cf_filename, std::ios::out | std::ios::app | std::ios::binary);
   _offset_file.open(_offset_filename, std::ios::out | std::ios::app | std::ios::binary);

   _Addr addr;
   addr.len = len;

   _cf_file.seekp(0, std::ios::end);
   addr.offset = _cf_file.tellp();

   _cf_file.write(data, len);

   _offset_file.seekp(idx * sizeof(addr));
   _offset_file.write((char *)&addr, sizeof(addr));
   _offset_file.flush();

   if (addr.len < 0)
      addr.len = addr.len;

   if (sizeof(addr) <= 0)
      addr.len = addr.len;

   _cf_file.flush();
}

void CfStorage::remove (int idx)
{

}
