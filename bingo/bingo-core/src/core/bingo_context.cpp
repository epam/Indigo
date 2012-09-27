/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "core/bingo_context.h"
#include "base_cpp/scanner.h"
#include "core/bingo_error.h"
#include "molecule/elements.h"

// const char * bingo_version_string = "1.7-beta3";

TL_DEF(BingoContext, PtrArray<BingoContext>, _instances);

OsLock BingoContext::_instances_lock;

IMPL_ERROR(BingoContext, "bingo context");

BingoContext::BingoContext (int id_)
{
   id = id_;
   reset();
   treat_x_as_pseudoatom.setName("treat_x_as_pseudoatom");
   ignore_closing_bond_direction_mismatch.setName("ignore_closing_bond_direction_mismatch");
}

void BingoContext::reset ()
{
   cmf_dict.reset();

   nthreads = 0;
   treat_x_as_pseudoatom = false;
   ignore_closing_bond_direction_mismatch = false;

   tautomer_rules_ready = false;
   fp_parameters_ready = false;
   atomic_mass_map_ready = false;
   treat_x_as_pseudoatom.reset();
   ignore_closing_bond_direction_mismatch.reset();
}


BingoContext::~BingoContext ()
{
}

void BingoContext::remove (int id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<BingoContext>, _instances);
   int i;

   for (i = 0; i < _instances.size(); i++)
      if (_instances[i]->id == id)
         break;

   //if (i == _instances.size())
   //   throw Error("remove(): context #%d not found", id);

   if (i != _instances.size())
      _instances.remove(i);
}

BingoContext * BingoContext::_get (int id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<BingoContext>, _instances);

   for (int i = 0; i < _instances.size(); i++)
      if (_instances[i]->id == id)
         return _instances[i];

   return 0;
}

BingoContext * BingoContext::existing (int id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<BingoContext>, _instances);

   for (int i = 0; i < _instances.size(); i++)
      if (_instances[i]->id == id)
         return _instances[i];

   throw Error("context #%d not found", id);
}

BingoContext * BingoContext::get (int id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<BingoContext>, _instances);

   for (int i = 0; i < _instances.size(); i++)
      if (_instances[i]->id == id)
         return _instances[i];

   return &_instances.add(new BingoContext(id));
}

void bingoGetTauCondition (const char *list_ptr, int &aromaticity, Array<int> &label_list)
{
   if (isdigit(*list_ptr))
   {
      if (*list_ptr == '1')
         aromaticity = 1;
      else if (*list_ptr == '0')
         aromaticity = 0;
      else
         throw BingoError("Bad tautomer configuration string format");
      list_ptr++;
   } else {
      aromaticity = -1;
   }

   label_list.clear();

   QS_DEF(Array<char>, buf);
   buf.clear();

   while (*list_ptr != 0)
   {
      if (isalpha((unsigned)*list_ptr))
         buf.push(*list_ptr);
      else if (*list_ptr == ',')
      {
         buf.push(0);
         label_list.push(Element::fromString(buf.ptr()));
         buf.clear();
      } else
         throw BingoError("Bad label list format");
      list_ptr++;
   }

   buf.push(0);
   label_list.push(Element::fromString(buf.ptr()));
}

void bingoGetName (Scanner &scanner, Array<char> &result)
{
   QS_DEF(Array<char>, str);
   bool single_line = Scanner::isSingleLine(scanner);

   result.clear();

   if (single_line)
   {
      scanner.readLine(str, true);
      int idx = str.find('|');
      int idx2 = 0;

      if (idx >= 0)
      {
         int tmp = str.find(idx + 1, str.size(), '|');

         if (tmp > 0)
            idx2 = tmp + 1;
      }

      BufferScanner scan2(str.ptr() + idx2);

      while (!scan2.isEOF())
      {
         if (isspace(scan2.readChar()))
            break;
      }
      scan2.skipSpace();
      if (!scan2.isEOF())
         scan2.readLine(result, false);
   }
   else
   {
      scanner.readLine(result, false);
      if (result.size() >= 4 && strncmp(result.ptr(), "$RXN", 4) == 0)
         scanner.readLine(result, false);
   }
}
