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

#include "render_cdxml.h"

#include "render_params.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cdxml_saver.h"

using namespace indigo;

// Molecule position on the page
struct Pos
{
   // Structure min and max coordinates
   Vec2f str_min, str_max;

   // Offset and size on the page
   Vec2f page_offset;
   Vec2f size;

   // Final offset for the coordinates
   Vec2f offset, title_offset;
};



void RenderParamCdxmlInterface::getBounds (Molecule &mol, Vec2f &min, Vec2f &max)
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
}

void RenderParamCdxmlInterface::render (RenderParams& params)
{
   MoleculeCdxmlSaver saver(*params.rOpt.output);

   Array<BaseMolecule*> mols;
   if (params.mols.size() != 0)
      for (int i = 0; i < params.mols.size(); ++i)
         mols.push(params.mols[i]);
   else
      mols.push(params.mol.get());

   Vec2f offset(0, 0);
   Array<float> column_widths;
   column_widths.resize(params.cnvOpt.gridColumnNumber);
   column_widths.fill(0);

   for (int i = 0; i < mols.size(); ++i)
   {
      int column = i % params.cnvOpt.gridColumnNumber;
      Vec2f min, max;
      getBounds(mols[i]->asMolecule(), min, max);

      float width = max.x - min.x;
      column_widths[column] = __max(width, column_widths[column]);
   }

   float x_margins_base = 1.1f, y_margins_base = 1.1f;
   float x_grid_base = 1.5f;

   Array<float> column_offset;
   column_offset.resize(params.cnvOpt.gridColumnNumber);
   column_offset[0] = params.cnvOpt.marginX / 10.0f + x_margins_base;
   for (int i = 1; i < params.cnvOpt.gridColumnNumber; i++)
      column_offset[i] = column_offset[i - 1] + column_widths[i - 1] + x_grid_base;

   float row_y_offset = params.cnvOpt.marginY / 10.0f + y_margins_base;
   int last_row = 0;
   float max_y = 0;

   saver.beginDocument();

   Array<Pos> positions;
   positions.resize(mols.size());

   float title_y = 0;

   // Get each structure bounds 
   for (int i = 0; i < mols.size(); ++i)
   {
      Pos &p = positions[i];

      int column = i % params.cnvOpt.gridColumnNumber;

      getBounds(mols[i]->asMolecule(), p.str_min, p.str_max);

      p.page_offset.x = column_offset[column];
      p.page_offset.y = row_y_offset;

      p.size.diff(p.str_max, p.str_min);
      
      p.offset.x = p.page_offset.x - p.str_min.x + (column_widths[column] - p.size.x) / 2;
      p.offset.y = -p.page_offset.y - p.str_max.y;

      p.title_offset.x = p.page_offset.x + p.size.x / 2 + (column_widths[column] - p.size.x) / 2;
      p.title_offset.y = -p.page_offset.y - p.size.y - 1.0f;

      if (i < params.titles.size())
      {
         const Array<char> &title = params.titles[i];
         if (title.size() > 0)
         {
            int lines = title.count('\n') + 1;
            float title_height = lines * 0.3f;
            max_y = __max(max_y, row_y_offset + p.size.y + title_height + 1.0f);
         }
      }

      max_y = __max(max_y, row_y_offset + p.size.y);

      int next_row = (i + 1) / params.cnvOpt.gridColumnNumber;
      if (last_row != next_row)
      {
         row_y_offset = max_y + 1.0f;
         last_row = next_row;
      }
   }

   if (params.cnvOpt.comment.size() > 0)
   {
      int lines = params.cnvOpt.comment.count('\n') + 1;
      float comment_height = lines * 0.3f;
      max_y += 0.3f;
      title_y = max_y;
      max_y += comment_height;
   }

   MoleculeCdxmlSaver::Bounds b;
   b.min.set(0, 0);

   float w = column_offset[params.cnvOpt.gridColumnNumber - 1] + column_widths[params.cnvOpt.gridColumnNumber - 1];
   w += x_margins_base + params.cnvOpt.marginX / 10.0f;

   b.max.set(w, max_y + y_margins_base);
   saver.beginPage(&b);

   for (int i = 0; i < mols.size(); ++i)
   {
      Pos &p = positions[i];
      saver.saveMoleculeFragment(mols[i]->asMolecule(), p.offset);
      
      if (i < params.titles.size())
      {
         const Array<char> &title = params.titles[i];
         if (title.size() > 0)
            saver.addText(p.title_offset, title.ptr());
      }
   }

   if (params.cnvOpt.comment.size() > 0)
   {
      Vec2f pos(b.max.x / 2, -title_y);
      saver.addText(pos, params.cnvOpt.comment.ptr());
   }

   saver.endPage();
   saver.endDocument();
}
