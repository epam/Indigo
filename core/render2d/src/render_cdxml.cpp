/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "render_cdxml.h"

#include "base_cpp/output.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cdxml_saver.h"
#include "reaction/reaction.h"
#include "reaction/reaction_cdxml_saver.h"
#include "render_params.h"

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

void _getBounds(RenderParams& params, BaseMolecule& mol, Vec2f& min, Vec2f& max, float& scale)
{
    // Compute average bond length
    float avg_bond_length = 1;
    if (mol.edgeCount() > 0)
    {
        float bond_length_sum = 0;
        for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
        {
            const Edge& edge = mol.getEdge(i);
            const Vec3f& p1 = mol.getAtomXyz(edge.beg);
            const Vec3f& p2 = mol.getAtomXyz(edge.end);
            bond_length_sum += Vec3f::dist(p1, p2);
        }
        avg_bond_length = bond_length_sum / mol.edgeCount();
    }

    float bond_length = 1;
    if (params.cnvOpt.bondLength > 0)
        bond_length =
            UnitsOfMeasure::convertToPx(params.cnvOpt.bondLength, params.cnvOpt.bondLengthUnit, params.rOpt.ppi) / LayoutOptions::DEFAULT_BOND_LENGTH_PX;

    scale = bond_length / avg_bond_length;

    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        Vec3f& p = mol.getAtomXyz(i);
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
int _findReverse(int from, int to, const Array<char>& _array, char value)
{
    for (int i = to - 1; i >= from; i--)
    {
        if (_array[i] == value)
            return i;
    }
    return -1;
}
int _getLongestLineXml(const Array<char>& line)
{
    int longest_line = 0;
    if (line.size() > 0)
    {
        int start = 0;
        while (start < line.size() - 1)
        {
            int next = line.find(start + 1, line.size(), '\n');
            if (next == -1)
                next = line.size() - 1;
            int st = _findReverse(start + 1, next - 1, line, '>');
            if (st == -1)
            {
                st = start;
            }

            longest_line = std::max(next - st, longest_line);

            start = next;
        }
    }
    return longest_line;
}
int _getLongestLine(const Array<char>& line)
{
    int longest_line = 0;
    if (line.size() > 0)
    {
        int start = 0;
        while (start < line.size())
        {
            int next = line.find(start + 1, line.size(), '\n');
            if (next == -1)
                next = line.size();

            longest_line = std::max(next - start, longest_line);

            start = next;
        }
    }
    return longest_line;
}

void RenderParamCdxmlInterface::render(RenderParams& params)
{
    if (params.rmode == RENDER_MOL)
        _renderMols(params);
    else if (params.rmode == RENDER_RXN)
        _renderRxns(params);
}

void RenderParamCdxmlInterface::_renderRxns(RenderParams& params)
{
    ReactionCdxmlSaver saver(*params.rOpt.output);
    if (params.rxns.size() != 0)
        for (int i = 0; i < params.rxns.size(); ++i)
            saver.saveReaction(*params.rxns[i]);
    else if (params.rxn)
        saver.saveReaction(*params.rxn);
}

void RenderParamCdxmlInterface::_renderMols(RenderParams& params)
{
    MoleculeCdxmlSaver saver(*params.rOpt.output);

    Array<BaseMolecule*> mols;
    Array<int> ids;

    if (params.mols.size() != 0)
        for (int i = 0; i < params.mols.size(); ++i)
            mols.push(params.mols[i]);
    else if (params.mol.get() != 0)
        mols.push(params.mol.get());

    Array<float> column_widths;
    column_widths.resize(params.cnvOpt.gridColumnNumber);
    column_widths.fill(0);

    Array<float> title_widths;
    title_widths.resize(mols.size());
    title_widths.fill(0);

    Array<float> key_widths;
    key_widths.resize(mols.size());
    key_widths.fill(0);

    Array<float> prop_widths;
    prop_widths.resize(mols.size());
    prop_widths.fill(0);

    Array<Pos> positions;
    positions.resize(mols.size());

    Array<float> title_heights;
    title_heights.resize(mols.size());
    title_heights.fill(0);

    for (int mol_idx = 0; mol_idx < mols.size(); ++mol_idx)
    {
        int column = mol_idx % params.cnvOpt.gridColumnNumber;

        Pos& p = positions[mol_idx];
        _getBounds(params, mols[mol_idx]->asMolecule(), p.str_min, p.str_max, p.scale);

        float width = p.str_max.x - p.str_min.x;

        // Check titles width
        if (mol_idx < params.titles.size())
        {
            const Array<char>& title = params.titles[mol_idx];

            if (title.size() > 0)
            {
                int longest_line = _getLongestLine(title);

                // On average letters has width 6
                float letter_width = params.rOpt.titleFontFactor / 1.5f;

                float title_width = longest_line * letter_width / MoleculeCdxmlSaver::SCALE;
                title_widths[mol_idx] = title_width;
                width = std::max(width, title_width);
            }
        }
        if (params.rOpt.cdxml_context.get() != NULL)
        {
            RenderCdxmlContext& context = *params.rOpt.cdxml_context;
            if (context.enabled)
            {
                RenderCdxmlContext::PropertyData& data = context.property_data.at(mol_idx);
                float letter_width = context.propertyFontSize / 1.5f;
                int longest_line = _getLongestLineXml(data.propertyName);

                key_widths[mol_idx] = longest_line * letter_width / MoleculeCdxmlSaver::SCALE;

                longest_line += _getLongestLineXml(data.propertyValue);
                float prop_width = longest_line * letter_width / MoleculeCdxmlSaver::SCALE;
                prop_widths[mol_idx] = prop_width;

                width = std::max(width, prop_width);
            }
        }

        column_widths[column] = std::max(width, column_widths[column]);
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
    for (int mol_idx = 0; mol_idx < mols.size(); ++mol_idx)
    {
        Pos& p = positions[mol_idx];

        int column = mol_idx % params.cnvOpt.gridColumnNumber;
        int row = mol_idx / params.cnvOpt.gridColumnNumber;

        p.page_offset.x = column_offset[column];
        p.page_offset.y = row_y_offset;

        p.size.diff(p.str_max, p.str_min);
        p.all_size = p.size;

        if (mol_idx < params.titles.size())
        {
            const Array<char>& title = params.titles[mol_idx];
            if (title.size() > 0)
            {
                int lines = title.count('\n') + 1;
                float letter_height = params.rOpt.titleFontFactor / MoleculeCdxmlSaver::SCALE;
                // float title_height = lines * saver.textLineHeight();
                // p.all_size.y += title_height + saver.textLineHeight(); // Add blank line
                float title_height = lines * letter_height;
                title_heights[mol_idx] = title_height;
                p.all_size.y += title_height + letter_height; // Add blank line
            }
        }
        if (params.rOpt.cdxml_context.get() != NULL)
        {
            RenderCdxmlContext& context = *params.rOpt.cdxml_context;
            if (context.enabled)
            {
                RenderCdxmlContext::PropertyData& data = context.property_data.at(mol_idx);
                int lines = data.propertyName.count('\n') + 1;

                float letter_height = params.rOpt.titleFontFactor / MoleculeCdxmlSaver::SCALE;
                float prop_height = lines * letter_height;
                p.all_size.y += prop_height + letter_height; // Add blank line
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
            mol_idx = row * params.cnvOpt.gridColumnNumber - 1;
            row_moved = row;
            continue;
        }

        p.offset.x = p.page_offset.x - p.str_min.x + (column_widths[column] - p.size.x) / 2;
        p.offset.y = -p.page_offset.y - p.str_max.y;

        p.title_offset_y = -p.page_offset.y - p.size.y - 1.0f;

        max_y = std::max(max_y, p.page_offset.y + p.all_size.y);

        int next_row = (mol_idx + 1) / params.cnvOpt.gridColumnNumber;
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

    Array<char> font_attr;
    ArrayOutput font_out(font_attr);

    font_out.printf("<s size=\"%f\"", params.rOpt.titleFontFactor);

    if (params.rOpt.cdxml_context.get() != NULL)
    {
        RenderCdxmlContext& context = *params.rOpt.cdxml_context;
        if (context.fonttable.size() > 0)
        {
            saver.addFontTable(context.fonttable.ptr());
        }
        if (context.colortable.size() > 0)
        {
            saver.addColorTable(context.colortable.ptr());
        }
        if (context.titleFont.size() > 0)
        {
            font_out.printf(" font=\"%s\"", context.titleFont.ptr());
        }
        if (context.titleFace.size() > 0)
        {
            font_out.printf(" face=\"%s\"", context.titleFace.ptr());
        }
    }
    font_out.printf(">");
    font_attr.push(0);

    // if (params.rOtitleFont.size() > 0) {
    //   font_out.printf("id=\"5\" charset=\"iso-8859-1\" name=\"%s\"", params.cnvOpt.titleFont.ptr());
    //   font_attr.push(0);
    //   saver.addFontTable(font_attr.ptr());
    //   /*
    //   * Set font as id 5 always
    //   */

    //   font_attr.clear();
    //   if (params.rOpt.titleFontFactor > 1)
    //      font_out.printf(" font=\"5\" size=\"%.0f\"", params.rOpt.titleFontFactor);
    //   else
    //      font_out.printf(" font=\"5\"");
    //} else {
    //   if (params.rOpt.titleFontFactor > 1)
    //      font_out.printf(" size=\"%.0f\"", params.rOpt.titleFontFactor);
    //}
    // font_attr.push(0);
    saver.beginPage(&b);
    Array<char> title_font;

    for (int mol_idx = 0; mol_idx < mols.size(); ++mol_idx)
    {
        int column = mol_idx % params.cnvOpt.gridColumnNumber;

        Pos& p = positions[mol_idx];
        Vec2f offset = p.offset;
        offset.scale(1 / p.scale);
        saver.saveMoleculeFragment(mols[mol_idx]->asMolecule(), offset, p.scale);

        if (mol_idx < params.titles.size())
        {
            const Array<char>& title = params.titles[mol_idx];

            if (title.size() > 0)
            {
                title_font.clear();
                title_font.readString(font_attr.ptr(), false);
                // Get title bounding box
                float x = params.cnvOpt.titleAlign.getAnchorPoint(p.page_offset.x, column_widths[column], title_widths[mol_idx]);

                const char* alignment_str = "";
                MultilineTextLayout alignment = params.cnvOpt.titleAlign;
                if (alignment.inbox_alignment == MultilineTextLayout::Center)
                    alignment_str = "Center";
                if (alignment.inbox_alignment == MultilineTextLayout::Left)
                    alignment_str = "Left";
                if (alignment.inbox_alignment == MultilineTextLayout::Right)
                    alignment_str = "Right";

                Vec2f title_offset(x, p.title_offset_y);
                title_font.appendString(title.ptr(), true);
                title_font.appendString("</s>", true);

                saver.addCustomText(title_offset, alignment_str, params.rOpt.titleFontFactor, title_font.ptr());
            }
        }
        if (params.rOpt.cdxml_context.get() != NULL)
        {
            RenderCdxmlContext& context = *params.rOpt.cdxml_context;
            if (context.enabled)
            {
                RenderCdxmlContext::PropertyData& data = context.property_data.at(mol_idx);
                float prop_width = prop_widths[mol_idx];
                float key_width = key_widths[mol_idx];
                float prop_offset_y = p.title_offset_y - title_heights[mol_idx];
                float x = params.cnvOpt.titleAlign.getAnchorPoint(p.page_offset.x, column_widths[column], prop_width);

                float prop_offset_key = prop_width * 0.5f;
                float prop_offset_val = prop_offset_key - (prop_width - key_width);
                if (context.keyAlignment == RenderCdxmlContext::ALIGNMENT_LEFT)
                {
                    Vec2f title_offset_key(x - prop_offset_key, prop_offset_y);
                    Vec2f title_offset_val(x + prop_offset_val, prop_offset_y);
                    saver.addCustomText(title_offset_key, "Left", context.propertyFontSize, data.propertyName.ptr());
                    saver.addCustomText(title_offset_val, "Left", context.propertyFontSize, data.propertyValue.ptr());
                }
                else
                {
                    Vec2f title_offset_key(x + prop_offset_val, prop_offset_y);
                    Vec2f title_offset_val(x + prop_offset_val, prop_offset_y);
                    saver.addCustomText(title_offset_key, "Right", context.propertyFontSize, data.propertyName.ptr());
                    saver.addCustomText(title_offset_val, "Left", context.propertyFontSize, data.propertyValue.ptr());
                }
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
