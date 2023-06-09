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

#include "base_cpp/properties_map.h"
#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "render_cdxml.h"

#include "indigo-renderer.h"
#include "indigo_array.h"
#include "indigo_internal.h"
#include "indigo_io.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "indigo_renderer_internal.h"
#include "option_manager.h"

// #define INDIGO_DEBUG

#ifdef INDIGO_DEBUG
#include <iostream>
#endif

using namespace indigo;

static _SessionLocalContainer<IndigoRenderer> indigo_renderer_self;

IndigoRenderer& indigoRendererGetInstance()
{
    IndigoRenderer& inst = indigo_renderer_self.getLocalCopy();
    inst.validate();
    return inst;
}

void IndigoRenderer::init()
{
    setOptionsHandlers();
    renderParams.clear();
}

#define __min3(a, b, c) (std::min(a, std::min(b, c)))
#define __max3(a, b, c) (std::max(a, std::max(b, c)))

#define CHECKRGB(r, g, b)                                                                                                                                      \
    if (__min3(r, g, b) < 0 || __max3(r, g, b) > 1.0 + 1e-6)                                                                                                   \
    throw IndigoError("Some of the color components are out of range [0..1]")

typedef RedBlackStringMap<int, false> StringIntMap;

IndigoRenderer::IndigoRenderer()
{
    indigo_id = TL_GET_SESSION_ID();
}

IndigoRenderer::~IndigoRenderer()
{
}

#define SET_POSITIVE_FLOAT_OPTION(option, error)                                                                                                               \
    [](float value) {                                                                                                                                          \
        if (value <= 0.0f)                                                                                                                                     \
            throw IndigoError(error);                                                                                                                          \
        option = value;                                                                                                                                        \
    },                                                                                                                                                         \
        [](float& value) { value = option; }

#define CHECK_AND_SETTER_GETTER_COLOR_OPTION(option)                                                                                                           \
    [](float r, float g, float b) {                                                                                                                            \
        CHECKRGB(r, g, b);                                                                                                                                     \
        option.set(r, g, b);                                                                                                                                   \
    },                                                                                                                                                         \
        [](float& r, float& g, float& b) {                                                                                                                     \
            r = option.x;                                                                                                                                      \
            g = option.y;                                                                                                                                      \
            b = option.z;                                                                                                                                      \
        }

DINGO_MODE indigoRenderMapOutputFormat(const char* format)
{
    std::string format_string(format);
    DINGO_MODE mode = MODE_NONE;
    if (format_string == "pdf")
    {
        mode = MODE_PDF;
    }
    else if (format_string == "png")
    {
        mode = MODE_PNG;
    }
    else if (format_string == "svg")
    {
        mode = MODE_SVG;
    }
    else if (format_string == "emf")
    {
        mode = MODE_EMF;
    }
    else if (format_string == "cdxml")
    {
        mode = indigo::MODE_CDXML;
    }
    return mode;
}

const char* indigoRenderOutputFormatToString(DINGO_MODE mode)
{
    switch (mode)
    {
    case MODE_PDF:
        return "pdf";
    case MODE_PNG:
        return "png";
    case MODE_SVG:
        return "svg";
    case MODE_EMF:
        return "emf";
    case MODE_CDXML:
        return "cdxml";
    default:
        return "none";
    }
}

void indigoRenderSetOutputFormat(const char* format)
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    rp.rOpt.mode = indigoRenderMapOutputFormat(format);
}

void indigoRenderGetOutputFormat(Array<char>& value)
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    const char* mode = indigoRenderOutputFormatToString(rp.rOpt.mode);
    value.readString(mode, true);
}

void indigoRenderSetStereoStyle(const char* mode)
{
    std::string mode_string(mode);
    STEREO_STYLE result;
    if (mode_string == "ext")
    {
        result = STEREO_STYLE_EXT;
    }
    else if (mode_string == "old")
    {
        result = STEREO_STYLE_OLD;
    }
    else if (mode_string == "none")
    {
        result = STEREO_STYLE_NONE;
    }
    else
    {
        throw IndigoError("Invalid stereo style, should be 'ext', 'old' or 'none'");
    }
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    rp.rOpt.stereoMode = result;
}

void indigoRenderGetStereoStyle(Array<char>& value)
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    switch (rp.rOpt.stereoMode)
    {
    case STEREO_STYLE_EXT:
        value.readString("ext", true);
        break;
    case STEREO_STYLE_OLD:
        value.readString("old", true);
        break;
    case STEREO_STYLE_NONE:
        value.readString("none", true);
        break;
    }
}

void indigoRenderSetLabelMode(const char* mode)
{
    std::string mode_string(mode);
    LABEL_MODE result;
    if (mode_string == "none")
    {
        result = LABEL_MODE_NONE;
    }
    else if (mode_string == "hetero")
    {
        result = LABEL_MODE_HETERO;
    }
    else if (mode_string == "terminal-hetero")
    {
        result = LABEL_MODE_TERMINAL_HETERO;
    }
    else if (mode_string == "all")
    {
        result = LABEL_MODE_ALL;
    }
    else
    {
        throw IndigoError("Invalid label mode, should be 'none', 'hetero', 'terminal-hetero' or 'all'");
    }
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    rp.rOpt.labelMode = result;
}

void indigoRenderGetLabelMode(Array<char>& value)
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    switch (rp.rOpt.labelMode)
    {
    case LABEL_MODE_NONE:
        value.readString("none", true);
        break;
    case LABEL_MODE_HETERO:
        value.readString("hetero", true);
        break;
    case LABEL_MODE_TERMINAL_HETERO:
        value.readString("terminal-hetero", true);
        break;
    case LABEL_MODE_ALL:
        value.readString("all", true);
        break;
    }
}

void indigoRenderSetCatalystsPlacement(const char* mode)
{
    int result;
    std::string mode_string(mode);
    if (mode_string == "above")
    {
        result = 0;
    }
    else if (mode_string == "above_and_below")
    {
        result = 1;
    }
    else
    {
        throw IndigoError("Unknown agent placement map, should be 'above' or 'above_and_below'");
    }

    RenderParams& rp = indigoRendererGetInstance().renderParams;
    rp.rOpt.agentsBelowArrow = result != 0;
}

void indigoRenderGetCatalystsPlacement(Array<char>& value)
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    if (rp.rOpt.agentsBelowArrow)
        value.readString("above-and-below", true);
    else
        value.readString("above", true);
}

void indigoRenderSetSuperatomMode(const char* mode)
{
    // std::string mode_string(mode);
    // int result;
    // if (mode_string == "expand")
    //{
    //     result = 0;
    // }
    // else if (mode_string == "collapse")
    //{
    //     result = 1;
    // }
    // else
    //{
    //     throw IndigoError("Invalid label mode, should be 'expand' or 'collapse'");
    // }
    // RenderParams& rp = indigoRendererGetInstance().renderParams;
    // rp.rOpt.collapseSuperatoms = result != 0;
}

void indigoRenderGetSuperatomMode(Array<char>& value)
{
    // RenderParams& rp = indigoRendererGetInstance().renderParams;
    // if (rp.rOpt.collapseSuperatoms)
    //     value.readString("collapse", true);
    // else
    //     value.readString("expand", true);
}

static MultilineTextLayout _parseTextLayout(const char* text)
{
    // Try to read as float for compatibility with previous versions
    BufferScanner scanner(text);
    float val;
    if (scanner.tryReadFloat(val))
    {
        const float eps = 1e-6f;
        if (fabs(val) < eps)
            text = "left";
        else if (fabs(val - 0.5f) < eps)
            text = "center";
        else if (fabs(val - 1.0f) < eps)
            text = "right";
        else
            throw IndigoError("Alignment allow only 0.0, 0.5, or 1.0 values");
    }

    if (strcasecmp(text, "left") == 0)
        return MultilineTextLayout(MultilineTextLayout::Left, MultilineTextLayout::Left);
    else if (strcasecmp(text, "right") == 0)
        return MultilineTextLayout(MultilineTextLayout::Right, MultilineTextLayout::Right);
    else if (strcasecmp(text, "center") == 0)
        return MultilineTextLayout(MultilineTextLayout::Center, MultilineTextLayout::Center);
    else if (strcasecmp(text, "center-left") == 0)
        return MultilineTextLayout(MultilineTextLayout::Center, MultilineTextLayout::Left);
    else if (strcasecmp(text, "center-right") == 0)
        return MultilineTextLayout(MultilineTextLayout::Center, MultilineTextLayout::Right);
    else
        throw IndigoError("Option value is invalid");
}

static void layoutToText(const MultilineTextLayout& layout, Array<char>& value)
{
    switch (layout.bbox_alignment)
    {
    case MultilineTextLayout::Left:
        value.readString("left", true);
        break;
    case MultilineTextLayout::Right:
        value.readString("right", true);
        break;
    case MultilineTextLayout::Center:
        switch (layout.inbox_alignment)
        {
        case MultilineTextLayout::Left:
            value.readString("center-left", true);
            break;
        case MultilineTextLayout::Right:
            value.readString("center-right", true);
            break;
        case MultilineTextLayout::Center:
            value.readString("center", true);
            break;
        }
        break;
    }
}

void indigoRenderSetCommentAlignment(const char* text)
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    rp.cnvOpt.commentAlign = _parseTextLayout(text);
}

void indigoRenderGetCommentAlignment(Array<char>& value)
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    layoutToText(rp.cnvOpt.commentAlign, value);
}

void indigoRenderSetTitleAlignment(const char* text)
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    rp.cnvOpt.titleAlign = _parseTextLayout(text);
}

void indigoRenderGetTitleAlignment(Array<char>& value)
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    layoutToText(rp.cnvOpt.titleAlign, value);
}

void indigoRenderSetCommentPosition(const char* pos)
{
    std::string pos_string(pos);
    COMMENT_POS result;
    if (pos_string == "top")
    {
        result = COMMENT_POS_TOP;
    }
    else if (pos_string == "bottom")
    {
        result = COMMENT_POS_BOTTOM;
    }
    else
    {
        throw IndigoError("Invalid comment position, should be 'top' or 'bottom'");
    }
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    rp.cnvOpt.commentPos = result;
}

void indigoRenderGetCommentPosition(Array<char>& value)
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    if (rp.cnvOpt.commentPos == COMMENT_POS_TOP)
        value.readString("top", true);
    else
        value.readString("bottom", true);
}

RenderCdxmlContext& getCdxmlContext()
{
    RenderParams& rp = indigoRendererGetInstance().renderParams;
    if (rp.rOpt.cdxml_context.get() == NULL)
    {
        rp.rOpt.cdxml_context = std::make_unique<RenderCdxmlContext>();
    }
    return *rp.rOpt.cdxml_context;
}

void indigoRenderSetCdxmlPropertiesKeyAlignment(const char* value)
{
    RenderCdxmlContext& context = getCdxmlContext();
    if (strcasecmp(value, "left") == 0)
        context.keyAlignment = RenderCdxmlContext::ALIGNMENT_LEFT;
    else if (strcasecmp(value, "right") == 0)
        context.keyAlignment = RenderCdxmlContext::ALIGNMENT_RIGHT;
    else
        throw IndigoError("Option value alignment is invalid");
}

void indigoRenderGetCdxmlPropertiesKeyAlignment(Array<char>& value)
{
    RenderCdxmlContext& context = getCdxmlContext();
    if (context.keyAlignment == RenderCdxmlContext::ALIGNMENT_LEFT)
        value.readString("left", true);
    else
        value.readString("right", true);
}

CEXPORT int indigoRendererInit(qword id)
{
#ifdef INDIGO_DEBUG
    std::stringstream ss;
    ss << "IndigoRenderer(" << id << ")";
    std::cout << ss.str() << std::endl;
#endif
    INDIGO_BEGIN_STATIC
    {
        auto& context = indigo_renderer_self.createOrGetLocalCopy(id);
        context.init();
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoRendererDispose(const qword id)
{
#ifdef INDIGO_DEBUG
    std::stringstream ss;
    ss << "~IndigoRenderer(" << id << ")";
    std::cout << ss.str() << std::endl;
#endif
    INDIGO_BEGIN_STATIC
    {
        indigo_renderer_self.removeLocalCopy(id);
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoRender(int object, int output)
{
    INDIGO_BEGIN
    {
        RenderParams& rp = indigoRendererGetInstance().renderParams;
        // If there are molecules/reactions in the arrays then current call will
        // rendere a grid -> needs to clear it
        rp.clearArrays();
        rp.smart_layout = self.smart_layout;

        IndigoObject& obj = self.getObject(object);

        if (IndigoBaseMolecule::is(obj))
        {
            if (obj.getBaseMolecule().isQueryMolecule())
                rp.mol.reset(new QueryMolecule());
            else
                rp.mol.reset(new Molecule());
            rp.mol->clone_KeepIndices(self.getObject(object).getBaseMolecule());
            rp.rmode = RENDER_MOL;
        }
        else if (IndigoBaseReaction::is(obj))
        {
            if (obj.getBaseReaction().isQueryReaction())
                rp.rxn.reset(new QueryReaction());
            else
                rp.rxn.reset(new Reaction());
            rp.rxn->clone(self.getObject(object).getBaseReaction(), 0, 0, 0);
            rp.rmode = RENDER_RXN;
        }
        else
        {
            throw IndigoError("The object provided should be a molecule, a reaction or an array of such");
        }

        IndigoObject& out = self.getObject(output);
        if (out.type == IndigoHDCOutput::HDC_OUTPUT)
        {
            IndigoHDCOutput& hdcOut = (IndigoHDCOutput&)self.getObject(output);
            rp.rOpt.hdc = hdcOut.dc;
            rp.rOpt.mode = hdcOut.prn ? MODE_PRN : MODE_HDC;
        }
        else if (out.type == IndigoObject::OUTPUT)
        {
            rp.rOpt.output = &IndigoOutput::get(out);
        }
        else
        {
            throw IndigoError("Invalid output object type");
        }
        RenderParamInterface::render(rp);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoRenderGrid(int objects, int* refAtoms, int nColumns, int output)
{
    INDIGO_BEGIN
    {
        RenderParams& rp = indigoRendererGetInstance().renderParams;
        rp.clearArrays();

        PtrArray<IndigoObject>& objs = IndigoArray::cast(self.getObject(objects)).objects;
        if (rp.rOpt.cdxml_context.get() != NULL)
        {
            RenderCdxmlContext& context = *rp.rOpt.cdxml_context;
            context.property_data.clear();
        }
        if (IndigoBaseMolecule::is(*objs[0]))
        {
            for (int i = 0; i < objs.size(); ++i)
            {
                if (objs[i]->getBaseMolecule().isQueryMolecule())
                    rp.mols.add(new QueryMolecule());
                else
                    rp.mols.add(new Molecule());
                Array<char>& title = rp.titles.push();
                if (objs[i]->getProperties().contains(rp.cnvOpt.titleProp.ptr()))
                    title.copy(objs[i]->getProperties().valueBuf(rp.cnvOpt.titleProp.ptr()));

                if (rp.rOpt.mode == DINGO_MODE::MODE_CDXML)
                {
                    if (rp.rOpt.cdxml_context.get() != NULL)
                    {

                        RenderCdxmlContext& context = *rp.rOpt.cdxml_context;
                        RenderCdxmlContext::PropertyData& data = context.property_data.push();

                        auto& properties = objs[i]->getProperties();
                        if (context.propertyNameCaption.size() > 0 && context.propertyValueCaption.size() > 0)
                            if (properties.contains(context.propertyNameCaption.ptr()))
                            {
                                if (properties.contains(context.propertyValueCaption.ptr()))
                                {
                                    data.propertyName.readString(properties.at(context.propertyNameCaption.ptr()), true);
                                    data.propertyValue.readString(properties.at(context.propertyValueCaption.ptr()), true);
                                }
                            }
                    }
                }

                rp.mols.top()->clone_KeepIndices(objs[i]->getBaseMolecule());
                rp.rmode = RENDER_MOL;
            }
        }
        else if (IndigoBaseReaction::is(*objs[0]))
        {
            for (int i = 0; i < objs.size(); ++i)
            {
                if (objs[i]->getBaseReaction().isQueryReaction())
                    rp.rxns.add(new QueryReaction());
                else
                    rp.rxns.add(new Reaction());
                Array<char>& title = rp.titles.push();
                if (objs[i]->getProperties().contains(rp.cnvOpt.titleProp.ptr()))
                    title.copy(objs[i]->getProperties().valueBuf(rp.cnvOpt.titleProp.ptr()));

                rp.rxns.top()->clone(objs[i]->getBaseReaction(), 0, 0, 0);
                rp.rmode = RENDER_RXN;
            }
        }
        else
        {
            throw IndigoError("The array elements should be molecules or reactions");
        }

        if (refAtoms != NULL)
        {
            rp.refAtoms.copy(refAtoms, objs.size());
        }

        rp.cnvOpt.gridColumnNumber = nColumns;

        bool hasNonemptyTitles = false;
        for (int i = 0; i < rp.titles.size(); ++i)
        {
            if (rp.titles[i].size() > 0)
            {
                hasNonemptyTitles = true;
                break;
            }
        }
        if (!hasNonemptyTitles)
            rp.titles.clear();

        IndigoObject& out = self.getObject(output);
        if (out.type == IndigoHDCOutput::HDC_OUTPUT)
        {
            IndigoHDCOutput& hdcOut = (IndigoHDCOutput&)self.getObject(output);
            rp.rOpt.hdc = hdcOut.dc;
            rp.rOpt.mode = hdcOut.prn ? MODE_PRN : MODE_HDC;
        }
        else if (out.type == IndigoObject::OUTPUT)
        {
            rp.rOpt.output = &IndigoOutput::get(out);
        }
        else
        {
            throw IndigoError("Invalid output object type");
        }
        RenderParamInterface::render(rp);

        // Release memory for arrays with molecules/reactions
        rp.clearArrays();

        return 1;
    }
    INDIGO_END(-1);
}

DINGO_MODE indigoRenderGuessOutputFormat(const char* filename)
{
    const char* ext = strrchr(filename, '.');
    if (ext == NULL)
        return MODE_NONE;

    return indigoRenderMapOutputFormat(ext + 1);
}

CEXPORT int indigoRenderToFile(int object, const char* filename)
{
    int f = indigoWriteFile(filename);

    if (f == -1)
        return -1;

    RenderParams& rp = indigoRendererGetInstance().renderParams;
    DINGO_MODE setMode = rp.rOpt.mode;
    rp.rOpt.mode = (setMode == MODE_NONE) ? indigoRenderGuessOutputFormat(filename) : setMode;
    int res = indigoRender(object, f);
    rp.rOpt.mode = setMode;

    indigoFree(f);
    return res;
}

CEXPORT int indigoRenderGridToFile(int objects, int* refAtoms, int nColumns, const char* filename)
{
    int f = indigoWriteFile(filename);

    if (f == -1)
        return -1;

    RenderParams& rp = indigoRendererGetInstance().renderParams;
    DINGO_MODE setMode = rp.rOpt.mode;
    rp.rOpt.mode = (setMode == MODE_NONE) ? indigoRenderGuessOutputFormat(filename) : setMode;
    int res = indigoRenderGrid(objects, refAtoms, nColumns, f);
    rp.rOpt.mode = setMode;

    indigoFree(f);
    return res;
}

CEXPORT int indigoRenderReset()
{
    INDIGO_BEGIN
    {
        IndigoRenderer& rp = indigoRendererGetInstance();
        rp.init();
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT void indigoRenderResetOptions()
{
    indigoRendererGetInstance().init();
    getCdxmlContext().clear();
}

CEXPORT int indigoRenderWriteHDC(void* hdc, int printingHdc)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoHDCOutput(hdc, printingHdc != 0));
    }
    INDIGO_END(-1);
}

void IndigoRenderer::setOptionsHandlers()
{
    if (!options_set)
    {
        auto mgr = sf::xlock_safe_ptr(indigoGetOptionManager(indigo_id));

#define rp indigoRendererGetInstance().renderParams
#define cdxmlContext getCdxmlContext()

        mgr->setOptionHandlerInt("render-comment-offset", SETTER_GETTER_INT_OPTION(rp.cnvOpt.commentOffset));
        mgr->setOptionHandlerInt("render-image-width", SETTER_GETTER_INT_OPTION(rp.cnvOpt.width));
        mgr->setOptionHandlerInt("render-image-height", SETTER_GETTER_INT_OPTION(rp.cnvOpt.height));
        mgr->setOptionHandlerInt("render-image-max-width", SETTER_GETTER_INT_OPTION(rp.cnvOpt.maxWidth));
        mgr->setOptionHandlerInt("render-image-max-height", SETTER_GETTER_INT_OPTION(rp.cnvOpt.maxHeight));

        mgr->setOptionHandlerString("render-output-format", indigoRenderSetOutputFormat, indigoRenderGetOutputFormat);

        mgr->setOptionHandlerString("render-label-mode", indigoRenderSetLabelMode, indigoRenderGetLabelMode);
        mgr->setOptionHandlerString("render-comment", SETTER_GETTER_STR_OPTION(rp.cnvOpt.comment));
        mgr->setOptionHandlerString("render-comment-position", indigoRenderSetCommentPosition, indigoRenderGetCommentPosition);
        mgr->setOptionHandlerString("render-stereo-style", indigoRenderSetStereoStyle, indigoRenderGetStereoStyle);
        mgr->setOptionHandlerString("render-catalysts-placement", indigoRenderSetCatalystsPlacement, indigoRenderGetCatalystsPlacement);
        mgr->setOptionHandlerString("render-superatom-mode", indigoRenderSetSuperatomMode, indigoRenderGetSuperatomMode);
        mgr->setOptionHandlerString("render-atom-color-property", SETTER_GETTER_STR_OPTION(rp.rOpt.atomColorProp));

        mgr->setOptionHandlerBool("render-coloring", SETTER_GETTER_BOOL_OPTION(rp.rOpt.atomColoring));
        mgr->setOptionHandlerBool("render-valences-visible", SETTER_GETTER_BOOL_OPTION(rp.rOpt.showValences));
        mgr->setOptionHandlerBool("render-atom-ids-visible", SETTER_GETTER_BOOL_OPTION(rp.rOpt.showAtomIds));
        mgr->setOptionHandlerBool("render-bond-ids-visible", SETTER_GETTER_BOOL_OPTION(rp.rOpt.showBondIds));
        mgr->setOptionHandlerBool("render-atom-bond-ids-from-one", SETTER_GETTER_BOOL_OPTION(rp.rOpt.atomBondIdsFromOne));
        mgr->setOptionHandlerBool("render-highlight-thickness-enabled", SETTER_GETTER_BOOL_OPTION(rp.rOpt.highlightThicknessEnable));
        mgr->setOptionHandlerBool("render-highlight-color-enabled", SETTER_GETTER_BOOL_OPTION(rp.rOpt.highlightColorEnable));
        mgr->setOptionHandlerBool("render-center-double-bond-when-stereo-adjacent", SETTER_GETTER_BOOL_OPTION(rp.rOpt.centerDoubleBondWhenStereoAdjacent));
        mgr->setOptionHandlerBool("render-implicit-hydrogens-visible", SETTER_GETTER_BOOL_OPTION(rp.rOpt.implHVisible));
        mgr->setOptionHandlerBool("render-highlighted-labels-visible", SETTER_GETTER_BOOL_OPTION(rp.rOpt.highlightedLabelsVisible));
        mgr->setOptionHandlerBool("render-bold-bond-detection", SETTER_GETTER_BOOL_OPTION(rp.rOpt.boldBondDetection));

        mgr->setOptionHandlerFloat("render-bond-length", SETTER_GETTER_FLOAT_OPTION(rp.cnvOpt.bondLength));
        mgr->setOptionHandlerFloat("render-relative-thickness", SET_POSITIVE_FLOAT_OPTION(rp.relativeThickness, "relative thickness must be positive"));
        mgr->setOptionHandlerFloat("render-bond-line-width", SET_POSITIVE_FLOAT_OPTION(rp.bondLineWidthFactor, "bond line width factor must be positive"));
        mgr->setOptionHandlerFloat("render-comment-font-size", SETTER_GETTER_FLOAT_OPTION(rp.rOpt.commentFontFactor));
        mgr->setOptionHandlerString("render-comment-alignment", indigoRenderSetCommentAlignment, indigoRenderGetCommentAlignment);
        mgr->setOptionHandlerFloat("render-comment-spacing", SETTER_GETTER_FLOAT_OPTION(rp.rOpt.commentSpacing));

        mgr->setOptionHandlerColor("render-background-color", SETTER_GETTER_COLOR_OPTION(rp.rOpt.backgroundColor));
        mgr->setOptionHandlerColor("render-base-color", SETTER_GETTER_COLOR_OPTION(rp.rOpt.baseColor));
        mgr->setOptionHandlerColor("render-highlight-color", CHECK_AND_SETTER_GETTER_COLOR_OPTION(rp.rOpt.highlightColor));
        mgr->setOptionHandlerColor("render-aam-color", CHECK_AND_SETTER_GETTER_COLOR_OPTION(rp.rOpt.aamColor));
        mgr->setOptionHandlerColor("render-comment-color", CHECK_AND_SETTER_GETTER_COLOR_OPTION(rp.rOpt.commentColor));
        mgr->setOptionHandlerColor("render-data-sgroup-color", CHECK_AND_SETTER_GETTER_COLOR_OPTION(rp.rOpt.dataGroupColor));

        mgr->setOptionHandlerXY("render-image-size", SETTER_GETTER_XY_OPTION(rp.cnvOpt.width, rp.cnvOpt.height));
        mgr->setOptionHandlerXY("render-hdc-offset", SETTER_GETTER_XY_OPTION(rp.cnvOpt.xOffset, rp.cnvOpt.yOffset));
        mgr->setOptionHandlerXY("render-margins", SETTER_GETTER_XY_OPTION(rp.cnvOpt.marginX, rp.cnvOpt.marginY));

        mgr->setOptionHandlerXY("render-grid-margins", SETTER_GETTER_XY_OPTION(rp.cnvOpt.gridMarginX, rp.cnvOpt.gridMarginY));
        mgr->setOptionHandlerFloat("render-grid-title-spacing", SETTER_GETTER_FLOAT_OPTION(rp.rOpt.titleSpacing));
        mgr->setOptionHandlerString("render-grid-title-alignment", indigoRenderSetTitleAlignment, indigoRenderGetTitleAlignment);
        mgr->setOptionHandlerFloat("render-grid-title-font-size", SETTER_GETTER_FLOAT_OPTION(rp.rOpt.titleFontFactor));
        mgr->setOptionHandlerString("render-grid-title-property", SETTER_GETTER_STR_OPTION(rp.cnvOpt.titleProp));
        mgr->setOptionHandlerInt("render-grid-title-offset", SETTER_GETTER_INT_OPTION(rp.cnvOpt.titleOffset));

        mgr->setOptionHandlerBool("render-cdxml-properties-enabled", SETTER_GETTER_BOOL_OPTION(cdxmlContext.enabled));
        mgr->setOptionHandlerString("render-cdxml-properties-fonttable", SETTER_GETTER_STR_OPTION(cdxmlContext.fonttable));
        mgr->setOptionHandlerString("render-cdxml-properties-colortable", SETTER_GETTER_STR_OPTION(cdxmlContext.colortable));
        mgr->setOptionHandlerString("render-cdxml-properties-name-property", SETTER_GETTER_STR_OPTION(cdxmlContext.propertyNameCaption));
        mgr->setOptionHandlerString("render-cdxml-properties-value-property", SETTER_GETTER_STR_OPTION(cdxmlContext.propertyValueCaption));
        mgr->setOptionHandlerString("render-cdxml-properties-key-alignment", indigoRenderSetCdxmlPropertiesKeyAlignment,
                                    indigoRenderGetCdxmlPropertiesKeyAlignment);
        mgr->setOptionHandlerFloat("render-cdxml-properties-size", SETTER_GETTER_FLOAT_OPTION(cdxmlContext.propertyFontSize));
        mgr->setOptionHandlerString("render-cdxml-title-font", SETTER_GETTER_STR_OPTION(cdxmlContext.titleFont));
        mgr->setOptionHandlerString("render-cdxml-title-face", SETTER_GETTER_STR_OPTION(cdxmlContext.titleFace));

        mgr->setOptionHandlerVoid("reset-render-options", indigoRenderResetOptions);

        options_set = true;
    }
}
