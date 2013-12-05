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
   Vec2f size, all_size;

   // Final offset for the coordinates
   Vec2f offset;
   float title_offset_y;

   // Structure scaling coefficient
   float scale;
};

void _getBounds (RenderParams& params, BaseMolecule &mol, Vec2f &min, Vec2f &max, float &scale)
{
   // Compute average bond length
   float avg_bond_length = 1;
   if (mol.edgeCount() > 0)
   {
      float bond_length_sum = 0;
      for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
      {
         const Edge& edge = mol.getEdge(i);
         const Vec3f &p1 = mol.getAtomXyz(edge.beg);
         const Vec3f &p2 = mol.getAtomXyz(edge.end);
         bond_length_sum += Vec3f::dist(p1, p2);
      }
      avg_bond_length = bond_length_sum / mol.edgeCount();
   }

   float bond_length = 1;
   if (params.cnvOpt.bondLength > 0)
      bond_length = params.cnvOpt.bondLength / 100.0f;

   scale = bond_length / avg_bond_length;

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

   Array<float> title_widths;
   title_widths.resize(mols.size());
   title_widths.fill(0);

   Array<Pos> positions;
   positions.resize(mols.size());

   for (int i = 0; i < mols.size(); ++i)
   {
      int column = i % params.cnvOpt.gridColumnNumber;

      Pos &p = positions[i];
      _getBounds(params, mols[i]->asMolecule(), p.str_min, p.str_max, p.scale);

      float width = p.str_max.x - p.str_min.x;

      // Check titles width
      if (i < params.titles.size())
      {
         const Array<char> &title = params.titles[i];
         
         if (title.size() > 0)
         {
            int start = 0;
            int longest_line = 0;
            while (start < title.size())
            {
               int next = title.find(start + 1, title.size(), '\n');
               if (next == -1)
                  next = title.size();

               longest_line = __max(next - start, longest_line);

               start = next;
            }

            // On average letters has width 6
            float title_width = longest_line * 6.3f / 36.0f;
            title_widths[i] = title_width;
            width = __max(width, title_width);
         }
      }
      

      column_widths[column] = __max(width, column_widths[column]);
   }

   float x_margins_base = 1.1f, y_margins_base = 1.1f;
   float x_grid_base = 1.5f;

   Array<float> column_offset;
   column_offset.resize(params.cnvOpt.gridColumnNumber);
   column_offset[0] = params.cnvOpt.marginX / 10.0f + x_margins_base;
   for (int i = 1; i < params.cnvOpt.gridColumnNumber; i++)
      column_offset[i] = column_offset[i - 1] + column_widths[i - 1] + x_grid_base;

   float page_y_offset_base = params.cnvOpt.marginY / 10.0f + y_margins_base;
   float row_y_offset = page_y_offset_base;
   int last_row = 0;
   float max_y = 0;

   float title_y = 0;

   // Get each structure bounds 
   int row_moved = 0;
   for (int i = 0; i < mols.size(); ++i)
   {
      Pos &p = positions[i];

      int column = i % params.cnvOpt.gridColumnNumber;
      int row = i / params.cnvOpt.gridColumnNumber;

      p.page_offset.x = column_offset[column];
      p.page_offset.y = row_y_offset;

      p.size.diff(p.str_max, p.str_min);
      p.all_size = p.size;
      
      if (i < params.titles.size())
      {
         const Array<char> &title = params.titles[i];
         if (title.size() > 0)
         {
            int lines = title.count('\n') + 1;
            float title_height = lines * saver.textLineHeight();
            p.all_size.y += title_height + saver.textLineHeight(); // Add blank line
         }
      }

      // Check that the structure is fully on a single page
      int pbegin = (int)(p.page_offset.y / saver.pageHeight());
      int pend = (int)((p.page_offset.y + p.all_size.y) / saver.pageHeight());
      // Additional check that we didn't moved this row before
      if (pbegin != pend && row_moved != row)
      {
         // Update starting row_y_offset for the whole row and start this row again
         row_y_offset = (pbegin + 1) * saver.pageHeight() + page_y_offset_base;
         i = row * params.cnvOpt.gridColumnNumber - 1;
         row_moved = row;
         continue;
      }

      p.offset.x = p.page_offset.x - p.str_min.x + (column_widths[column] - p.size.x) / 2;
      p.offset.y = -p.page_offset.y - p.str_max.y;

      p.title_offset_y = -p.page_offset.y - p.size.y - 1.0f;

      max_y = __max(max_y, p.page_offset.y + p.all_size.y);

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
   saver.beginDocument(&b);
   saver.beginPage(&b);

   for (int i = 0; i < mols.size(); ++i)
   {
      int column = i % params.cnvOpt.gridColumnNumber;

      Pos &p = positions[i];
      Vec2f offset = p.offset;
      offset.scale(1 / p.scale);
      saver.saveMoleculeFragment(mols[i]->asMolecule(), offset, p.scale);
      
      if (i < params.titles.size())
      {
         const Array<char> &title = params.titles[i];
         if (title.size() > 0)
         {
            // Get title bounding box
            float x = params.cnvOpt.titleAlign.getAnchorPoint(p.page_offset.x, column_widths[column], title_widths[i]);

            const char *alignment_str = "";
            MultilineTextLayout alignment = params.cnvOpt.titleAlign;
            if (alignment.inbox_alignment == MultilineTextLayout::Center)
               alignment_str = "Center";
            if (alignment.inbox_alignment == MultilineTextLayout::Left)
               alignment_str = "Left";
            if (alignment.inbox_alignment == MultilineTextLayout::Right)
               alignment_str = "Right";

            Vec2f title_offset(x, p.title_offset_y);
            saver.addText(title_offset, title.ptr(), alignment_str);
         }
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
