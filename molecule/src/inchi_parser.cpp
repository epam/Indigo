/****************************************************************************
* Copyright (C) 2015 GGA Software Services LLC
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

#include "molecule/inchi_parser.h"

#include <string>

using namespace indigo;

InChICodeParser::InChICodeParser(const char *inchi_code)
: _mobileCount(0)
{
   std::string str(inchi_code);

   size_t pos = str.find("/h");
   // assert(pos != npos)
   unsigned num = 0, from = -1;
   bool isValid;
   int type = STATIC;
   for (pos += 2; pos < str.size() && str[pos] != '/'; ++pos)
   {
      isValid = false;
      switch (str[pos])
      {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         num = num * 10 + (str[pos] - '0');
         break;
      case ',':
         if (type == 0)
         {
            type = MOBILE;
            _mobileCount += (num? num: 1);
         }
         else
         {
            if (from == -1)
            {
               _hydrogens.push(num - 1);
               _types.push(type);
            }
            else
            {
               for (auto i = from; i <= num; ++i)
               {
                  _hydrogens.push(i - 1);
                  _types.push(type);
               }
               from = -1;
            }
         }
         num = 0;
         break;
      case '-':
         if (from != -1);//throw exception
         from = num;
         num = 0;
         break;
      case '(':
         if (pos < str.size() && str[pos + 1] == 'H')
         {
            ++pos;
            type = 0;
         }
         else
         {
            //throw exception
         }
         break;
      case ')':
         if (from == -1)
         {
            _hydrogens.push(num - 1);
            _types.push(type);
         }
         else
         {
            for (auto i = from; i <= num; ++i)
            {
               _hydrogens.push(i - 1);
               _types.push(type);
            }
            from = -1;
         }
         num = 0;
         type = STATIC;
         isValid = true;
         break;
      case 'H':
         if (from == -1)
         {
            _hydrogens.push(num - 1);
            _types.push(type);
         }
         else
         {
            for (auto i = from; i <= num; ++i)
            {
               _hydrogens.push(i - 1);
               _types.push(type);
            }
            from = -1;
         }
         num = 0;
         if (pos < str.size() && str[pos + 1] == ',')
         {
            ++pos;
         }
         else
         {
            isValid = true;
         }
         break;
      default:
         ;
      }
   }
   if (!isValid)
   {
      // throw exception
   }

}

int InChICodeParser::_nextElement(int type, int index)
{
   if (index == -1)
      index = 0;
   else
      ++index;

   for (; index != _hydrogens.size(); ++index)
   {
      if (_types[index] & type)
         break;
   }
   return index;
}
