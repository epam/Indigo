/****************************************************************************
 * Copyright (C) 2010-2013 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "bingo.h"

#include "indigo_internal.h"
#include "indigo_cpp.h"
#include "bingo_internal.h"

#include <stdio.h>
#include <string>

#include "base_cpp/ptr_pool.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/exception.h"

using namespace indigo;

// TODO: warning C4273: 'indigo::BingoException::BingoException' : inconsistent dll linkage 
IMPL_EXCEPTION(indigo, BingoException, "bingo");

class SearchObject;

class BingoContext
{
public:
   BingoContext ()
   {
      _data_file = NULL;
      _offset_file = NULL;
      _data_file_length = _records_count = -1;
   }

   ~BingoContext ()
   {
      _close();
   }

   void create (const char *location)
   {
      _openFiles(location, true);
   }

   void open (const char *location)
   {
      _openFiles(location, false);
   }

   int addRecord (const byte *buf, int length)
   {
      // Write data first
      fseek(_data_file, 0, SEEK_END);
      fwrite(buf, length, 1, _data_file);

      // Write offset information
      _RecordInfo info;
      info.offset = _data_file_length;
      info.length = length;

      fseek(_offset_file, 0, SEEK_END);
      fwrite(&info, sizeof(info), 1, _offset_file);

      _data_file_length += length;
      _records_count++;
      return _records_count - 1;
   }

   int countRecords ()
   {
      return _records_count;
   }

   void getRecord (int index, Array<byte> &buf)
   {
      if (index < 0 || index >= _records_count)
         throw BingoException("internal error: access to an invalid record %d out of %d", index, _records_count);
      fseek(_offset_file, index * sizeof(_RecordInfo), SEEK_SET);
      _RecordInfo info;
      fread(&info, sizeof(info), 1, _offset_file);

      buf.resize(info.length);
      fseek(_data_file, info.offset, SEEK_SET);
      fread(buf.ptr(), buf.size(), 1, _data_file);
   }

private:
   void _setLocation (const char *location)
   {
      _location = location;
      _data_filename = _location + "_data.dat";
      _offset_filename = _location + "_offset.dat";
   }

   void _openFiles (const char *location, bool new_database)
   {
      _setLocation(location);  

      const char *mode = new_database ? "w+" : "a+";

      _data_file = fopen(_data_filename.c_str(), mode);
      _offset_file = fopen(_offset_filename.c_str(), mode);
      fseek(_data_file, 0, SEEK_END);
      fseek(_offset_file, 0, SEEK_END);

      _data_file_length = ftell(_data_file);
      _records_count = ftell(_offset_file) / sizeof(_RecordInfo);

      if (_data_file == NULL || _offset_file == NULL)
      {
         _close();
         throw BingoException("Cannot %s file database at %s", new_database ? "create" : "open", location);
      }
   }

   void _close ()
   {
      if (_data_file != NULL)
         fclose(_data_file);
      if (_offset_file != NULL)
         fclose(_offset_file);
   }

   struct _RecordInfo
   {
      int offset, length;
   };

   std::string _location, _data_filename, _offset_filename;

   FILE *_data_file, *_offset_file;
   int _data_file_length, _records_count;
};

// Generic search object
class SearchObject
{
public:
   virtual bool next () = 0;
   virtual int currentIndex () = 0;
};

class SimpleSearchObject : public SearchObject
{
public:
   SimpleSearchObject (BingoContext &context) : _context(context) 
   {
      _index = -1;
   }

   virtual bool next ()
   {
      _index++;
      while (_index < _context.countRecords())
      {
         _context.getRecord(_index, _buf);

         _indigo_object = indigoUnserialize(_buf.ptr(), _buf.size());
         if (_tryCurrent())
            return true;
         _index++;
      }
      return false;
   }

   virtual int currentIndex ()
   {
      return _index;
   }

protected:
   virtual bool _tryCurrent () = 0;

   BingoContext &_context;
   int _index;

   IndigoAutoObj _indigo_object;
   Array<byte> _buf;
};

class SubSearchObject : public SimpleSearchObject
{
public:
   SubSearchObject (BingoContext &context, int query_obj_id) : SimpleSearchObject(context)
   {
      _query_obj_id = query_obj_id;
   }

protected:
   virtual bool _tryCurrent ()
   {
      IndigoAutoObj matcher = indigoSubstructureMatcher(_indigo_object, "");
      IndigoAutoObj match = indigoMatch(matcher, _query_obj_id);
      if (match != 0)
         return true;
      return false;
   }

private:
   int _query_obj_id;
};

class SimSearchObject : public SimpleSearchObject
{
public:
   SimSearchObject (BingoContext &context, int query_obj_id, float min, float max) : SimpleSearchObject(context)
   {
      _query_obj_id = query_obj_id;
      _min = min;
      _max = max;
   }

protected:
   virtual bool _tryCurrent ()
   {
      float sim = indigoSimilarity(_indigo_object, _query_obj_id, "");
      return _min <= sim && sim <= _max;
   }

private:
   int _query_obj_id;
   float _min, _max;
};

static PtrPool<BingoContext> _bingo_instances;
static PtrPool<SearchObject> _searches;

static int _bingoCreateOrLoadDatabaseFile (const char *location, const char *type, const char *options, bool create)
{
   AutoPtr<BingoContext> context(new BingoContext());

   if (create)
      context->create(location);
   else
      context->open(location);

   int db_id = _bingo_instances.add(context.release());

   return db_id;
}


CEXPORT int bingoCreateDatabaseFile (const char *location, const char *type, const char *options)
{
   INDIGO_BEGIN
   {
      return _bingoCreateOrLoadDatabaseFile(location, type, options, true);
   }
   INDIGO_END(-1);
}

CEXPORT int bingoLoadDatabaseFile (const char *location)
{
   INDIGO_BEGIN
   {
      return _bingoCreateOrLoadDatabaseFile(location, NULL, NULL, false);
   }
   INDIGO_END(-1);
}

CEXPORT int bingoCloseDatabase (int db)
{
   INDIGO_BEGIN
   {
      _bingo_instances.remove(db);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int bingoInsertRecordObj (int db, int obj_id)
{
   INDIGO_BEGIN
   {
      BingoContext &bingo = _bingo_instances.ref(db);

      byte *buf;
      int size;
      if (indigoSerialize(obj_id, &buf, &size) < 0)
         return -1; // Exception has already been written into buffer

      return bingo.addRecord(buf, size);
   }
   INDIGO_END(-1);
}

CEXPORT int bingoDeleteRecord (int db, int index)
{
   INDIGO_BEGIN
   {
      throw BingoException("bingoDeleteRecord is not implemented yet");
   }
   INDIGO_END(-1);
}

CEXPORT int bingoSearchSub (int db, int query_obj, const char *options)
{
   INDIGO_BEGIN
   {
      BingoContext &bingo = _bingo_instances.ref(db);
      return _searches.add(new SubSearchObject(bingo, query_obj));
   }
   INDIGO_END(-1);
}

CEXPORT int bingoSearchSim (int db, int query_obj, float min, float max, const char *options)
{
   INDIGO_BEGIN
   {
      BingoContext &bingo = _bingo_instances.ref(db);
      return _searches.add(new SimSearchObject(bingo, query_obj, min, max));
   }
   INDIGO_END(-1);
}

CEXPORT int bingoEndSearch (int search_obj)
{
   INDIGO_BEGIN
   {
      _searches.remove(search_obj);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int bingoNext (int search_obj)
{
   INDIGO_BEGIN
   {
      return _searches.ref(search_obj).next();
   }
   INDIGO_END(-1);
}

CEXPORT int bingoGetCurrentIndex (int search_obj)
{
   INDIGO_BEGIN
   {
      return _searches.ref(search_obj).currentIndex();
   }
   INDIGO_END(-1);
}

CEXPORT int bingoGetObject (int search_obj)
{
   INDIGO_BEGIN
   {
      throw BingoException("bingoGetObject is not implemented yet");
   }
   INDIGO_END(-1);
}
