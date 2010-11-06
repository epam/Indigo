/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __render_reaction_h__
#define __render_reaction_h__

#include "render_base.h"

namespace indigo {

class Reaction;
class ReactionHighlighting;

class ReactionRender : public RenderBase {
public:
   ReactionRender (RenderContext& rc);
   virtual ~ReactionRender ();
   void setReaction (BaseReaction* r);
   void setReactionHighlighting (ReactionHighlighting* r);

   DEF_ERROR("reaction render");
protected:
   enum ITEM_TYPE_RXN {ITEM_TYPE_RXN_PLUS = ITEM_TYPE_BASE_MAX, ITEM_TYPE_RXN_ARROW, 
      ITEM_TYPE_RXN_BEGIN_ARROW, ITEM_TYPE_RXN_END_ARROW, ITEM_TYPE_RXN_SPACE, ITEM_TYPE_RXN_MAX};

   virtual void _initLayout ();
   void _drawMol (const Metalayout::LayoutItem& item);
   void _drawPlus ();
   void _drawArrow (float length = -1);
   virtual BaseMolecule& _getMol (int id);

   Metalayout::LayoutItem& _pushMol (Metalayout::LayoutLine& line, int id, bool catalyst = false);
   Metalayout::LayoutItem& _pushSymbol (Metalayout::LayoutLine& line, int type);
   Metalayout::LayoutItem& _pushSpace (Metalayout::LayoutLine& line, float size);

   static void cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context);

private:
   ReactionHighlighting* _highlighting;
   BaseReaction* _r;
   float _ax;
};

}

#endif //__render_reaction_h__
