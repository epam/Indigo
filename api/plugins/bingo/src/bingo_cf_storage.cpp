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

   _cf_file.open(cf_filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
   _offset_file.open(offset_filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
}

void CfStorage::load (const char *cf_filename, const char *offset_filename)
{
   _cf_filename = cf_filename;
   _offset_filename = offset_filename;

   _cf_file.open(cf_filename, std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
   _offset_file.open(offset_filename, std::ios::in | std::ios::out | std::ios::app | std::ios::binary);

   if (!_cf_file.is_open())
      throw Exception("cf storage file missed");

   if (!_offset_file.is_open())
      throw Exception("cf storage offset file missed");

   _Addr addr;
   int i = 0;
   while (_offset_file.read((char *)(&addr), sizeof(addr)))
   {
      char *buf = NULL;
      
      if (addr.len != -1)
      {
         buf = new char[addr.len];

         _cf_file.seekg(addr.offset);
         _cf_file.read(buf, addr.len);
      }

      _CfBuf &cf_buf = _cf_strings.push();
      
      cf_buf.buf.reset(buf);
      cf_buf.len = addr.len;

      if (i % 100000 == 0)
         std::cout << i << std::endl;
      i++;
   }

   _cf_file.close();
   _offset_file.close();
   _cf_file.open(cf_filename, std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
   _offset_file.open(offset_filename, std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
}

const char * CfStorage::get (int idx, int &len)
{
   if (_cf_strings[idx].len == 0)
   {
      len = -1;
      return 0;
   }

   len = (int)_cf_strings[idx].len;
   return _cf_strings[idx].buf.get();
}

void CfStorage::add (const char *data, int len, int idx)
{
   _Addr addr;
   addr.len = len;

   if (idx >= _cf_strings.size())
      _cf_strings.resize(idx + 1);

   _cf_strings[idx].buf.reset(new char[len]);
   memcpy(_cf_strings[idx].buf.get(), data, len);
   _cf_strings[idx].len = len;

   _cf_file.seekp(0, std::ios::end);
   addr.offset = (int)_cf_file.tellp();

   _cf_file.write(data, len);

   _offset_file.seekp(idx * sizeof(addr));
   _offset_file.write((char *)&addr, sizeof(addr));
   _offset_file.flush();

   if (sizeof(addr) <= 0)
      addr.len = addr.len;

   _cf_file.flush();
}

void CfStorage::remove (int idx)
{
   _cf_strings[idx].buf.release();
   _cf_strings[idx].len = -1;

   _Addr addr;
   addr.len = -1;

   _offset_file.seekp(idx * sizeof(addr) + sizeof(addr.offset));
   _offset_file.write((char *)&addr.len, sizeof(addr.len));
   _offset_file.flush();
}
