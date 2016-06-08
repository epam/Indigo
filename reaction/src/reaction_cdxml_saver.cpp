/****************************************************************************
 * Copyright (C) 2016 EPAM Systems
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

#include "reaction/reaction_cdxml_saver.h"
#include "base_cpp/output.h"
#include "reaction/reaction.h"
#include "molecule/molecule_cdxml_saver.h"

using namespace indigo;

// Molecule position on the page
struct Pos
{
   // Structure min and max coordinates
   Vec2f str_min, str_max;

   // Offset and size on the page
   Vec2f page_offset;
   Vec2f size, all_size;

   // Final offset for the coordinates
   Vec2f offset;
   float title_offset_y;

   // Structure scaling coefficient
   float scale;
};

void _getBounds (BaseMolecule &mol, Vec2f &min, Vec2f &max, float scale)
{
   for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
   {
      Vec3f &p = mol.getAtomXyz(i);
      Vec2f p2(p.x, p.y);

      if (i == mol.vertexBegin())
         min = max = p2;
      else
      {
         min.min(p2);
         max.max(p2);
      }
   }

   min.scale(scale);
   max.scale(scale);
}

IMPL_ERROR(ReactionCdxmlSaver, "reaction CDXML saver");

ReactionCdxmlSaver::ReactionCdxmlSaver (Output &output) : _output(output)
{
}

ReactionCdxmlSaver::~ReactionCdxmlSaver ()
{
}

void ReactionCdxmlSaver::saveReaction (BaseReaction &rxn)
{
   int i;
   Array<int> reactants_ids;
   Array<int> products_ids;
   ObjArray< Array<int> > nodes_ids;

   int arrow_id;

   MoleculeCdxmlSaver molsaver(_output);
   MoleculeCdxmlSaver::Bounds b;

   reactants_ids.clear();
   products_ids.clear();

   _generateCdxmlObjIds(rxn, reactants_ids, products_ids, nodes_ids, arrow_id);
   
   molsaver.beginDocument(NULL);
   molsaver.addDefaultFontTable();
   molsaver.addDefaultColorTable();
   molsaver.beginPage(NULL);

   Vec2f offset(0, 0);

   if (rxn.reactantsCount() > 0)
   {
      for (i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
      {
         molsaver.saveMoleculeFragment(rxn.getBaseMolecule(i), offset, 1, reactants_ids[i], nodes_ids[i]);
      }
   }
   if (rxn.productsCount() > 0)
   {
      for (i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
      {
         molsaver.saveMoleculeFragment(rxn.getBaseMolecule(i), offset, 1, products_ids[i], nodes_ids[i]);
      }
   }
/*
   if (rxn.catalystCount() > 0)
   {
      for (i = rxn.catalystBegin(); i != rxn.catalystEnd(); i = rxn.catalystNext(i))
      {
         molsaver.saveMoleculeFragment(rxn.getBaseMolecule(i), offset, 1);
      }
   }
*/
   _addPlusses(rxn, molsaver);

   _addArrow(rxn, molsaver, arrow_id);
   _addScheme(molsaver);
   _addStep(rxn, molsaver, reactants_ids, products_ids, nodes_ids, arrow_id);
   _closeScheme(molsaver);

   if (rxn.name.size() > 0)
   {
      _addTitle(rxn, molsaver);
   }

   molsaver.endPage();
   molsaver.endDocument();
}

void ReactionCdxmlSaver::_addPlusses (BaseReaction &rxn, MoleculeCdxmlSaver &molsaver)
{
   Vec2f offset(0, 0);

   if (rxn.reactantsCount() > 1)
   {
      int rcount = 1;
      for (auto i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
      {
         if (rcount < rxn.reactantsCount())
         {
            Vec2f min1, max1;
            Vec2f min2, max2;
            _getBounds (rxn.getBaseMolecule(i), min1, max1, 1.0);
            _getBounds (rxn.getBaseMolecule(rxn.reactantNext(i)), min2, max2, 1.0);
            offset.x = (max1.x + min2.x)/2;
            offset.y = (min1.y + max1.y)/2;

            molsaver.addText(offset, "+");
            rcount++;
         }
      }
   }

   if (rxn.productsCount() > 1)
   {
      int pcount = 1;
      for (auto i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
      {
         if (pcount < rxn.productsCount())
         {
            Vec2f min1, max1;
            Vec2f min2, max2;
            _getBounds (rxn.getBaseMolecule(i), min1, max1, 1.0);
            _getBounds (rxn.getBaseMolecule(rxn.productNext(i)), min2, max2, 1.0);
            offset.x = (max1.x + min2.x)/2;
            offset.y = (min1.y + max1.y)/2;

            molsaver.addText(offset, "+");
            pcount++;
         }
      }
   }
}

void ReactionCdxmlSaver::_addArrow (BaseReaction &rxn, MoleculeCdxmlSaver &molsaver, int arrow_id)
{
   int id = -1;
   Vec2f p1(0, 0);
   Vec2f p2(0, 0);
   PropertiesMap attrs;
   attrs.clear();

   Vec2f rmin, rmax;

   if (rxn.reactantsCount() > 0)
   {
      for (auto i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
      {
         Vec2f min, max;
         _getBounds (rxn.getBaseMolecule(i), min, max, 1.0);
         if (i == rxn.reactantBegin())
         {
            rmin = min; 
            rmax = max; 
         }
         else
         {
            rmin.min(min);
            rmax.max(max);
         }
      }
   }
   else
      return;

   Vec2f pmin, pmax;

   if (rxn.productsCount() > 0)
   {
      for (auto i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
      {
         Vec2f min, max;
         _getBounds (rxn.getBaseMolecule(i), min, max, 1.0);
         if (i == rxn.productBegin())
         {
            pmin = min; 
            pmax = max; 
         }
         else
         {
            pmin.min(min);
            pmax.max(max);
         }
      }
   }
   else
      return;

   if ((pmin.x - rmax.x) > 0)
   {
      p2.x = (rmax.x + pmin.x)/2 - (pmin.x - rmax.x)/4;
      p2.y = (rmin.y + rmax.y)/2;
   }
   else
   {
      p2.x = (rmax.x + pmin.x)/2 - 1.0;
      p2.y = (rmin.y + rmax.y)/2;
   }

   if ((pmin.x - rmax.x) > 0)
   {
      p1.x = (rmax.x + pmin.x)/2 + (pmin.x - rmax.x)/4;
      p1.y = (pmin.y + pmax.y)/2;
   }
   else
   {
      p1.x = (rmax.x + pmin.x)/2 + 1.0;
      p1.y = (pmin.y + pmax.y)/2;
   }

   Array<char> buf;
   ArrayOutput buf_out(buf);
   buf_out.printf("%d", arrow_id);
   buf.push(0);

   attrs.insert("id", buf.ptr());
   attrs.insert("GraphicType", "Line");
   attrs.insert("ArrowType", "FullHead");
   attrs.insert("HeadSize", "1000");

   molsaver.addGraphic (id, p1, p2, attrs);
}

void ReactionCdxmlSaver::_addScheme (MoleculeCdxmlSaver &molsaver)
{
   int id = -1;
   Array<char> name;
   PropertiesMap attrs;

   name.clear();
   attrs.clear();

   name.readString("scheme", true);
   molsaver.startCurrentElement(id, name, attrs);
}

void ReactionCdxmlSaver::_closeScheme (MoleculeCdxmlSaver &molsaver)
{
   molsaver.endCurrentElement();
}

void ReactionCdxmlSaver::_addStep (BaseReaction &rxn, MoleculeCdxmlSaver &molsaver, Array<int> &reactants_ids, Array<int> &products_ids,
        ObjArray< Array<int> > &nodes_ids, int arrow_id)
{
   int id = -1;
   Array<char> name;
   PropertiesMap attrs;

   name.clear();
   attrs.clear();

   name.readString("step", true);

   Array<char> buf;
   ArrayOutput buf_out(buf);
   for (auto i = 0; i < reactants_ids.size(); i++)
   {
      if (reactants_ids[i] > 0)
         buf_out.printf("%d ", reactants_ids[i]);
   }
   buf.pop();
   buf.push(0);
   attrs.insert("ReactionStepReactants", buf.ptr());

   buf.clear();
   for (auto i = 0; i < products_ids.size(); i++)
   {
      if (products_ids[i] > 0)
         buf_out.printf("%d ", products_ids[i]);
   }
   buf.pop();
   buf.push(0);
   attrs.insert("ReactionStepProducts", buf.ptr());

   buf.clear();
   buf_out.printf("%d", arrow_id);
   buf.push(0);
   attrs.insert("ReactionStepArrows", buf.ptr());

   buf.clear();
   for (auto i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
   {
      BaseMolecule &mol = rxn.getBaseMolecule(i);

      for (auto j = mol.vertexBegin(); j != mol.vertexEnd(); j = mol.vertexNext(j))
      {
         int aam = rxn.findAamNumber(&mol, j);
         if (aam > 0)
         {
            for (auto k = rxn.productBegin(); k != rxn.productEnd(); k = rxn.productNext(k))
            {
               int mapped_atom = rxn.findAtomByAAM(k, aam);
               if (mapped_atom != -1)
               {
                  buf_out.printf("%d %d ", nodes_ids[i][j], nodes_ids[k][mapped_atom]);
               }
            }
         }
      }
   }

   if (buf.size() > 1)
   {
      buf.pop();
      buf.push(0);
      attrs.insert("ReactionStepAtomMap", buf.ptr());
      attrs.insert("ReactionStepAtomMapManual", buf.ptr());
   }


   molsaver.addCustomElement(id, name, attrs);
}

void ReactionCdxmlSaver::_generateCdxmlObjIds (BaseReaction &rxn, Array<int> &reactants_ids, Array<int> &products_ids,
      ObjArray< Array<int> > &nodes_ids, int &arrow_id)
{
   reactants_ids.clear_resize(rxn.reactantEnd());
   reactants_ids.zerofill();

   products_ids.clear_resize(rxn.productEnd());
   products_ids.zerofill();

   int id = 0;

   for (auto i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
   {
      id++;
      reactants_ids[i] = id;

      BaseMolecule &mol = rxn.getBaseMolecule(i);

      nodes_ids.expand(i + 1);
      nodes_ids[i].clear_resize(mol.vertexEnd());
      nodes_ids[i].zerofill();

      for (auto j = mol.vertexBegin(); j != mol.vertexEnd(); j = mol.vertexNext(j))
      {
         id++;
         nodes_ids[i][j] = id;        
      }
   }

   for (auto i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
   {
      id++;
      products_ids[i] = id;

      BaseMolecule &mol = rxn.getBaseMolecule(i);

      nodes_ids.expand(i + 1);
      nodes_ids[i].clear_resize(mol.vertexEnd());
      nodes_ids[i].zerofill();

      for (auto j = mol.vertexBegin(); j != mol.vertexEnd(); j = mol.vertexNext(j))
      {
         id++;
         nodes_ids[i][j] = id;        
      }
   }

   arrow_id = id + 1;

   return;
}

void ReactionCdxmlSaver::_addTitle (BaseReaction &rxn, MoleculeCdxmlSaver &molsaver)
{
   Vec2f p(0, 0);
   PropertiesMap attrs;
   attrs.clear();

   Vec2f rmin, rmax;

   if (rxn.reactantsCount() > 0)
   {
      for (auto i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
      {
         Vec2f min, max;
         _getBounds (rxn.getBaseMolecule(i), min, max, 1.0);
         if (i == rxn.reactantBegin())
         {
            rmin = min; 
            rmax = max; 
         }
         else
         {
            rmin.min(min);
            rmax.max(max);
         }
      }
   }

   Vec2f pmin, pmax;

   if (rxn.productsCount() > 0)
   {
      for (auto i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
      {
         Vec2f min, max;
         _getBounds (rxn.getBaseMolecule(i), min, max, 1.0);
         if (i == rxn.productBegin())
         {
            pmin = min; 
            pmax = max; 
         }
         else
         {
            pmin.min(min);
            pmax.max(max);
         }
      }
   }

   p.x = (rmin.x + pmax.x)/2 - rxn.name.size() * 0.1;
   p.y = (rmax.y > pmax.y ? rmax.y : pmax.y) + 1.0;

   molsaver.addTitle (p, rxn.name.ptr());
}
