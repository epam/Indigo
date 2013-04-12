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

   _cf_outfile.open(_cf_filename, std::ios::out | std::ios::binary);
   _offset_outfile.open(_offset_filename, std::ios::out | std::ios::binary);
   
   _cf_infile.open(cf_filename, std::ios::in | std::ios::app | std::ios::binary);
   _offset_infile.open(offset_filename, std::ios::in | std::ios::app | std::ios::binary);
}

void CfStorage::load (const char *cf_filename, const char *offset_filename)
{
   _cf_filename = cf_filename;
   _offset_filename = offset_filename;

   _cf_outfile.open(_cf_filename, std::ios::out | std::ios::app | std::ios::binary);
   _offset_outfile.open(_offset_filename, std::ios::out | std::ios::app | std::ios::binary);
   
   _cf_infile.open(cf_filename, std::ios::in | std::ios::app | std::ios::binary);
   _offset_infile.open(offset_filename, std::ios::in | std::ios::app | std::ios::binary);
}

const char * CfStorage::get (int idx, int &len)
{
   _Addr addr;

   _offset_infile.seekg(idx * sizeof(addr));
   _offset_infile.read((char *)(&addr), sizeof(addr));

   _cf_infile.seekg(addr.offset);
   _cf_infile.read(_cur_cf_str, addr.len);

   len = addr.len;

   return _cur_cf_str; 
}

void CfStorage::add (const char *data, int len, int idx)
{
   _Addr addr;
   addr.len = len;

   _cf_outfile.seekp(0, std::ios::end);
   addr.offset = (int)_cf_outfile.tellp();

   _cf_outfile.write(data, len);

   _offset_outfile.seekp(idx * sizeof(addr));
   _offset_outfile.write((char *)&addr, sizeof(addr));
   _offset_outfile.flush();

   if (addr.len < 0)
      addr.len = addr.len;

   if (sizeof(addr) <= 0)
      addr.len = addr.len;

   _cf_outfile.flush();
}

void CfStorage::remove (int idx)
{

}
