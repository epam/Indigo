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

   _cf_outfile.open(cf_filename, std::ios::out | std::ios::binary);
   _offset_outfile.open(offset_filename, std::ios::out | std::ios::binary);
   
   _cf_infile.open(cf_filename, std::ios::in | std::ios::app | std::ios::binary);
   _offset_infile.open(offset_filename, std::ios::in | std::ios::app | std::ios::binary);
}

void CfStorage::load (const char *cf_filename, const char *offset_filename)
{
   _cf_filename = cf_filename;
   _offset_filename = offset_filename;

   _cf_outfile.open(cf_filename, std::ios::out | std::ios::app | std::ios::binary);
   _offset_outfile.open(offset_filename, std::ios::out | std::ios::app | std::ios::binary);
   
   _cf_infile.open(cf_filename, std::ios::in | std::ios::app | std::ios::binary);
   _offset_infile.open(offset_filename, std::ios::in | std::ios::app | std::ios::binary);

   if (!_cf_infile.is_open() || !_cf_outfile.is_open())
      throw Exception("cf storage file missed");

   if (!_offset_infile.is_open() || !_offset_outfile.is_open())
      throw Exception("cf storage offset file missed");

   _Addr addr;
   int i = 0;
   while (_offset_infile.read((char *)(&addr), sizeof(addr)))
   {
      char *buf = NULL;
      
      if (addr.len != -1)
      {
         buf = new char[addr.len];

         _cf_infile.seekg(addr.offset);
         _cf_infile.read(buf, addr.len);
      }

      _CfBuf &cf_buf = _cf_strings.push();
      
      cf_buf.buf.reset(buf);
      cf_buf.len = addr.len;

      if (i % 100000 == 0)
         std::cout << i << std::endl;
      i++;
   }

   _cf_infile.close();
   _offset_infile.close();
   _cf_infile.open(cf_filename, std::ios::in | std::ios::app | std::ios::binary);
   _offset_infile.open(offset_filename, std::ios::in | std::ios::app | std::ios::binary);
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

   _cf_outfile.seekp(0, std::ios::end);
   addr.offset = (int)_cf_outfile.tellp();

   _cf_outfile.write(data, len);

   _offset_outfile.seekp(idx * sizeof(addr));
   _offset_outfile.write((char *)&addr, sizeof(addr));
   _offset_outfile.flush();

   if (sizeof(addr) <= 0)
      addr.len = addr.len;

   _cf_outfile.flush();
}

void CfStorage::remove (int idx)
{
   _cf_strings[idx].buf.release();
   _cf_strings[idx].len = -1;

   _Addr addr;
   addr.len = -1;

   _offset_outfile.seekp(idx * sizeof(addr) + sizeof(addr.offset));
   _offset_outfile.write((char *)&addr.len, sizeof(addr.len));
   _offset_outfile.flush();
}
