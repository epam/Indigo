/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __molecule_cdxml_saver_h__
#define __molecule_cdxml_saver_h__

#include "math/algebra.h"


class TiXmlDocument;
class TiXmlElement;

namespace indigo {

class Molecule;
class Output;

class DLLEXPORT MoleculeCdxmlSaver
{
public:
   explicit MoleculeCdxmlSaver (Output &output);

   void saveMolecule (Molecule &mol);
   enum {
      BOND_LENGTH = 30
   };

   struct Bounds
   {
      Vec2f min, max;
   };

   void beginDocument (Bounds *bounds);
   void beginPage (Bounds *bounds);
   void addFontTable(const char* font);
   void addFontToTable(int id, const char* charset, const char* name);
   void addColorTable(const char* color);
   void addColorToTable(int id, int r, int g, int b);
   void saveMoleculeFragment (Molecule &mol, const Vec2f &offset, float scale);
   void addText (const Vec2f &pos, const char *text);
   void addText (const Vec2f &pos, const char *text, const char *alignment);
   void addCustomText(const Vec2f &pos, const char *alignment, float line_height, const char *text);
   void endPage ();
   void endDocument ();

   float pageHeight () const;
   float textLineHeight () const;

   DECL_ERROR;

private:
   Output   &_output;

   float _bond_length;
   int _pages_height;
   float _max_page_height;

   TiXmlDocument * _doc;
   TiXmlElement * _root;
   TiXmlElement * _page;
   TiXmlElement * _current;
   TiXmlElement * _fonttable;
   TiXmlElement * _colortable;

   MoleculeCdxmlSaver (const MoleculeCdxmlSaver &); // no implicit copy
};

}

#endif

