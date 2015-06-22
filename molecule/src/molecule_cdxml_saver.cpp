/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#include "base_cpp/output.h"
#include "molecule/molecule_cdxml_saver.h"
#include "molecule/molecule.h"
#include "molecule/elements.h"
#include "base_cpp/locale_guard.h"
#include "tinyxml.h"

using namespace indigo;

IMPL_ERROR(MoleculeCdxmlSaver, "molecule CMXML saver");

MoleculeCdxmlSaver::MoleculeCdxmlSaver (Output &output) : _output(output)
{
   _bond_length = BOND_LENGTH;
   _max_page_height = 64;
   _pages_height = 1;
   _doc = new TiXmlDocument();
   _root = new TiXmlElement("CDXML");
}

float MoleculeCdxmlSaver::pageHeight () const
{
   return _max_page_height;
}

float MoleculeCdxmlSaver::textLineHeight () const
{
   return 12.75f / _bond_length;
}

void MoleculeCdxmlSaver::beginDocument (Bounds *bounds)
{
   TiXmlDeclaration * decl = new TiXmlDeclaration("1.0", "UTF-8", "");
   _doc->LinkEndChild(decl);
   TiXmlUnknown * doctype = new TiXmlUnknown();
   doctype->SetValue("!DOCTYPE CDXML SYSTEM \"http://www.cambridgesoft.com/xml/cdxml.dtd\" ");
   _doc->LinkEndChild(doctype);

   QS_DEF(Array<char>, buf);
   ArrayOutput out(buf);
   out.printf("%f", _bond_length);
   buf.push(0);
   _root->SetAttribute("_bond_length", buf.ptr());
   _doc->LinkEndChild(_root);

   if (bounds != NULL)
   {
      // Generate MacPrintInfo according to the size
      // http://www.cambridgesoft.com/services/documentation/sdk/chemdraw/cdx/properties/MacPrintInfo.htm

      int dpi_logical = 72;
      int dpi_print = 600;

      float x_inch = bounds->max.x * _bond_length / dpi_logical + 1;
      float y_inch = bounds->max.y * _bond_length / dpi_logical + 1;

      int width = (int)(x_inch * dpi_print);
      int height = (int)(y_inch * dpi_print);

      // Add 1 to compensate margins = 36 points = 0.5 inches
      int max_height = (int)((_max_page_height * _bond_length / dpi_logical + 1) * dpi_print);
      if (height > max_height)
      {
         _pages_height = (int)ceil((float)height / max_height);
         height = max_height;
      }

      int mac_print_info[60] = {0};
      mac_print_info[0] = 3;  // magic number
      mac_print_info[2] = dpi_print;
      mac_print_info[3] = dpi_print;

      mac_print_info[6] = height;
      mac_print_info[7] = width;

      mac_print_info[10] = height;
      mac_print_info[11] = width;

      mac_print_info[12] = 871; // magic number

      mac_print_info[13] = height / 5; // magic scaling coeffient
      mac_print_info[14] = width / 5;

      mac_print_info[24] = 100; // horizontal scale, in percent
      mac_print_info[25] = 100; // Vertical scale, in percent


      _root->SetAttribute("PrintMargins", "36 36 36 36");

      buf.clear();
      for (int i = 0; i < NELEM(mac_print_info); i++)
      {
        out.printf("%04hx", (unsigned short)mac_print_info[i]);
      }
      buf.push(0);
      _root->SetAttribute("MacPrintInfo", buf.ptr());
   }
   _current = _root;
}

void MoleculeCdxmlSaver::beginPage (Bounds *bounds)
{
   _page = new TiXmlElement("page");
   _root->LinkEndChild(_page);
   _page->SetAttribute("HeightPages", _pages_height);
   _page->SetAttribute("WidthPages", 1);
   _current = _page;
}
void MoleculeCdxmlSaver::addFontTable(const char* font)
{
   if (font != NULL && strlen(font) > 0) 
   {
      _fonttable = new TiXmlElement("fonttable");
      _root->LinkEndChild(_fonttable);

      TiXmlUnknown * f = new TiXmlUnknown();
      _fonttable->LinkEndChild(f);

      QS_DEF(Array<char>, buf);
      ArrayOutput out(buf);
      buf.readString(&font[1], false);
      buf.remove(buf.size()-1);
      buf.push(0);
      f->SetValue(buf.ptr());
   }
}

void MoleculeCdxmlSaver::addFontToTable(int id, const char* charset, const char* name)
{
   TiXmlElement * font = new TiXmlElement("font");
   _fonttable->LinkEndChild(font);
   if (id > 0) 
      font->SetAttribute("id", id);
   font->SetAttribute("charset", charset);
   font->SetAttribute("name", name);
}

void MoleculeCdxmlSaver::addColorTable(const char* color)
{
   if (color != NULL && strlen(color) > 0)
   {
      _colortable = new TiXmlElement("colortable");
      _root->LinkEndChild(_colortable);

      addColorToTable(-1, 1, 1, 1);
      addColorToTable(-1, 0, 0, 0);
      addColorToTable(-1, 1, 0, 0);
      addColorToTable(-1, 1, 1, 0);
      addColorToTable(-1, 0, 1, 0);
      addColorToTable(-1, 0, 1, 1);
      addColorToTable(-1, 0, 0, 1);
      addColorToTable(-1, 1, 0, 1);

      TiXmlUnknown * c = new TiXmlUnknown();
      _colortable->LinkEndChild(c);

      QS_DEF(Array<char>, buf);
      ArrayOutput out(buf);
      buf.readString(&color[1], false);
      buf.remove(buf.size()-1);
      buf.push(0);
      c->SetValue(buf.ptr());
   }
}
void MoleculeCdxmlSaver::addColorToTable(int id, int r, int g, int b)
{
   TiXmlElement * color = new TiXmlElement("color");
   _colortable->LinkEndChild(color);
   if (id > 0) 
      color->SetAttribute("id", id);
   color->SetAttribute("r", r);
   color->SetAttribute("g", g);
   color->SetAttribute("b", b);
}


void MoleculeCdxmlSaver::saveMoleculeFragment (Molecule &mol, const Vec2f &offset, float structure_scale)
{
   float scale = structure_scale * _bond_length;

   LocaleGuard locale_guard;

   TiXmlElement * parent = _current;
   TiXmlElement * fragment = new TiXmlElement("fragment");
   _current->LinkEndChild(fragment);
   _current = fragment;

   bool have_hyz = mol.have_xyz;

   Vec2f min_coord, max_coord;

   if (mol.vertexCount() > 0)
   {
      for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
      {
         int atom_number = mol.getAtomNumber(i);
         int charge = mol.getAtomCharge(i);
         int radical = 0;

         TiXmlElement * node = new TiXmlElement("n");
         fragment->LinkEndChild(node);

         if (mol.isRSite(i))
         {
            node->SetAttribute("id", i + 1);
            node->SetAttribute("NodeType", "GenericNickname");
            node->SetAttribute("GenericNickname", "A");

            if (charge != 0)
               node->SetAttribute("Charge", charge);
         }
         else if (mol.isPseudoAtom(i))
         {
            node->SetAttribute("id", i + 1);
            node->SetAttribute("NodeType", "GenericNickname");
            node->SetAttribute("GenericNickname", mol.getPseudoAtom(i));

            if (charge != 0)
               node->SetAttribute("Charge", charge);
         }
         else 
         {
            node->SetAttribute("id", i + 1);
            node->SetAttribute("Element", atom_number);
            if (charge != 0)
               node->SetAttribute("Charge", charge);

            if (mol.getAtomIsotope(i) != 0)
               node->SetAttribute("Isotope", mol.getAtomIsotope(i));
  
  
            radical = mol.getAtomRadical_NoThrow(i, 0);
            if (radical != 0)
            {
               const char *radical_str = NULL;
               if (radical == RADICAL_DOUBLET)
                  radical_str = "Doublet";
               else if (radical == RADICAL_SINGLET)
                  radical_str = "Singlet";
               else
                  throw Error("Radical type %d is not supported", radical);
   
               node->SetAttribute("Radical", radical_str);
            }

            if (Molecule::shouldWriteHCount(mol, i))
            {
               int hcount;
   
               try
               {
                  hcount = mol.getAtomTotalH(i);
               }
               catch (Exception &)
               {
                  hcount = -1;
               }
   
               if (hcount >= 0)
                  node->SetAttribute("NumHydrogens", hcount);

            }
         }

         Vec3f pos3 = mol.getAtomXyz(i);
         Vec2f pos(pos3.x, pos3.y);

         pos.add(offset);
         if (i == mol.vertexBegin())
            min_coord = max_coord = pos;
         else
         {
            min_coord.min(pos);
            max_coord.max(pos);
         }

         pos.scale(scale);
         if (have_hyz)
         {
            QS_DEF(Array<char>, buf);
            ArrayOutput out(buf);
            out.printf("%f %f", pos.x, -pos.y);
            buf.push(0);
            node->SetAttribute("p", buf.ptr());
         }
         else
         {
            if (mol.stereocenters.getType(i) > MoleculeStereocenters::ATOM_ANY)
            {
               node->SetAttribute("Geometry", "Tetrahedral");

               const int *pyramid = mol.stereocenters.getPyramid(i);
               // 0 means atom absence
               QS_DEF(Array<char>, buf);
               ArrayOutput out(buf);
               out.printf("%d %d %d %d", pyramid[0] + 1, pyramid[1] + 1, pyramid[2] + 1, pyramid[3] + 1);
               buf.push(0);
               node->SetAttribute("BondOrdering", buf.ptr());
            }
         }

         if (mol.getVertex(i).degree() == 0 && atom_number == ELEM_C && charge == 0 && radical == 0)
         {
            TiXmlElement * t = new TiXmlElement("t");
            node->LinkEndChild(t);
         
            QS_DEF(Array<char>, buf);
            ArrayOutput out(buf);
            out.printf("%f %f", pos.x, -pos.y);
            buf.push(0);
            t->SetAttribute("p",buf.ptr());
            t->SetAttribute("Justification", "Center");
         
            TiXmlElement * s = new TiXmlElement("s");
            t->LinkEndChild(s);
            s->SetAttribute("font", 3);
            s->SetAttribute("size", 10);
            s->SetAttribute("face", 96);

            TiXmlText * txt = new TiXmlText("CH4");
            s->LinkEndChild(txt);
         }
         else if (mol.isRSite(i))
         {
            TiXmlElement * t = new TiXmlElement("t");
            node->LinkEndChild(t);
         
            QS_DEF(Array<char>, buf);
            ArrayOutput out(buf);
            out.printf("%f %f", pos.x, -pos.y);
            buf.push(0);
            t->SetAttribute("p", buf.ptr());
            t->SetAttribute("LabelJustification", "Left");
         
            TiXmlElement * s = new TiXmlElement("s");
            t->LinkEndChild(s);
            s->SetAttribute("font", 3);
            s->SetAttribute("size", 10);
            s->SetAttribute("face", 96);

			out.clear();
			out.printf("A");
			/*
			 * Skip charge since Chemdraw is pure. May be in future it will be fixed by Chemdraw
			*/
			/*if (charge != 0) {
				if (charge > 0) {
					out.printf("+%d", charge);
				}
				else {
					out.printf("-%d", charge);
				}
			}*/
			buf.push(0);

            TiXmlText * txt = new TiXmlText(buf.ptr());
            s->LinkEndChild(txt);
         }
         else if (mol.isPseudoAtom(i))
         {
            TiXmlElement * t = new TiXmlElement("t");
            node->LinkEndChild(t);
         
            QS_DEF(Array<char>, buf);
            ArrayOutput out(buf);
            out.printf("%f %f", pos.x, -pos.y);
            buf.push(0);
            t->SetAttribute("p", buf.ptr());
            t->SetAttribute("LabelJustification", "Left");
         
            TiXmlElement * s = new TiXmlElement("s");
            t->LinkEndChild(s);
            s->SetAttribute("font", 3);
            s->SetAttribute("size", 10);
            s->SetAttribute("face", 96);

			out.clear();

			out.printf("%s", mol.getPseudoAtom(i));
			/*
			* Skip charge since Chemdraw is pure. May be in future it will be fixed by Chemdraw
			*/
			/*if (charge != 0) {
				if (charge > 0) {
					out.printf("+%d", charge);
				}
				else {
					out.printf("-%d", charge);
				}
			}*/
			buf.push(0);
			TiXmlText * txt = new TiXmlText(buf.ptr());
            s->LinkEndChild(txt);
         }
      }
   }

   if (mol.edgeCount() > 0)
   {
      for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
      {
         const Edge &edge = mol.getEdge(i);

         TiXmlElement * bond = new TiXmlElement("b");
         fragment->LinkEndChild(bond);

         bond->SetAttribute("B", edge.beg + 1);
         bond->SetAttribute("E", edge.end + 1);

         int order = mol.getBondOrder(i);

         if (order == BOND_DOUBLE || order == BOND_TRIPLE)
            bond->SetAttribute("Order", order);
         else if (order == BOND_AROMATIC)
         {
            bond->SetAttribute("Order", "1.5");
            bond->SetAttribute("Display", "Dash");
            bond->SetAttribute("Display2", "Dash");
         }
         else
            ; // Do not write single bond order

         int dir = mol.getBondDirection(i);
         int parity = mol.cis_trans.getParity(i);

         if (mol.have_xyz && (dir == BOND_UP || dir == BOND_DOWN))
         {
            bond->SetAttribute("Display", (dir == BOND_UP) ? "WedgeBegin" : "WedgedHashBegin");
         }
         else if (!mol.have_xyz && parity != 0)
         {
            const int *subst = mol.cis_trans.getSubstituents(i);

            int s3 = subst[2] + 1, s4  = subst[3] + 1;
            if (parity == MoleculeCisTrans::TRANS)
            {
               int tmp;
               __swap(s3, s4, tmp);
            }
            QS_DEF(Array<char>, buf);
            ArrayOutput out(buf);
            out.printf("%d %d %d %d", subst[0] + 1, subst[1] + 1, s3, s4);
            buf.push(0);
            bond->SetAttribute("BondCircularOrdering", buf.ptr());
         }
      }
   }

   if (mol.isChrial())
   {
      Vec2f chiral_pos(max_coord.x, max_coord.y);
      Vec2f bbox(scale * chiral_pos.x, -scale * chiral_pos.y);

      TiXmlElement * graphic = new TiXmlElement("graphic");
      fragment->LinkEndChild(graphic);

      QS_DEF(Array<char>, buf);
      ArrayOutput out(buf);
      out.printf("%f %f %f %f", bbox.x, bbox.y, bbox.x, bbox.y);
      buf.push(0);
      graphic->SetAttribute("BoundingBox", buf.ptr());
      graphic->SetAttribute("GraphicType", "Symbol");
      graphic->SetAttribute("SymbolType", "Absolute");
      graphic->SetAttribute("FrameType", "None");

      _current = graphic;
      addText(chiral_pos, "Chiral");
      _current = fragment;
   }
   _current = parent;
}

void MoleculeCdxmlSaver::addText (const Vec2f &pos, const char *text)
{
   addText(pos, text, "Center");
}

void MoleculeCdxmlSaver::addText (const Vec2f &pos, const char *text, const char *alignment)
{
   TiXmlElement * t = new TiXmlElement("t");
   _current->LinkEndChild(t);

   QS_DEF(Array<char>, buf);
   ArrayOutput out(buf);
   out.printf("%f %f", _bond_length * pos.x, -_bond_length * pos.y);
   buf.push(0);
   t->SetAttribute("p", buf.ptr());
   t->SetAttribute("Justification", alignment);

   TiXmlElement * s = new TiXmlElement("s");
   t->LinkEndChild(s);
   TiXmlText * txt = new TiXmlText(text);
   s->LinkEndChild(txt);
}

void MoleculeCdxmlSaver::addCustomText(const Vec2f &pos, const char *alignment, float line_height, const char *text)
{
   TiXmlElement * t = new TiXmlElement("t");
   _current->LinkEndChild(t);

   QS_DEF(Array<char>, buf);
   ArrayOutput out(buf);
   out.printf("%f %f", _bond_length * pos.x, -_bond_length * pos.y);
   buf.push(0);
   t->SetAttribute("p", buf.ptr());
   t->SetAttribute("Justification", alignment);

   out.clear();
   out.printf("%f", line_height);
   buf.push(0);
   t->SetAttribute("LineHeight", buf.ptr());

   TiXmlUnknown * s = new TiXmlUnknown();
   buf.readString(text, false);
   if (buf.size() > 1) {
	  buf.remove(buf.size()-1);
	  buf.remove(0);
	  buf.push(0);
	  s->SetValue(buf.ptr());
	  t->LinkEndChild(s);
   }
   
}

void MoleculeCdxmlSaver::endPage ()
{
   _current = _root;
}

void MoleculeCdxmlSaver::endDocument ()
{
   TiXmlPrinter printer;
   _doc->Accept(&printer);
   _output.printf("%s", printer.CStr());
}


void MoleculeCdxmlSaver::saveMolecule (Molecule &mol)
{
   Vec3f min_coord, max_coord;
   if (mol.have_xyz)
   {
      for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
      {
         Vec3f &pos = mol.getAtomXyz(i);
         if (i == mol.vertexBegin())
            min_coord = max_coord = pos;
         else
         {
            min_coord.min(pos);
            max_coord.max(pos);
         }
      }

      // Add margins
      max_coord.add(Vec3f(1, 1, 1));
      min_coord.sub(Vec3f(1, 1, 1));
   }
   else
   {
      min_coord.set(0, 0, 0);
      max_coord.set(0, 0, 0);
   }

   beginDocument(NULL);
   beginPage(NULL);

   Vec2f offset(-min_coord.x, -max_coord.y);

   saveMoleculeFragment(mol, offset, 1);
   endPage();
   endDocument();
}
