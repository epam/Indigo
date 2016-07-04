package com.epam.indigo.controls;

import javax.swing.JTextPane;
import javax.swing.text.AbstractDocument;
import javax.swing.text.BoxView;
import javax.swing.text.Element;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledEditorKit;
import javax.swing.text.View;
import javax.swing.text.ViewFactory;

public class CenteredTextPane extends JTextPane
{
   public CenteredTextPane ()
   {
      setEditorKit(new VerticallyCenteredEditorKit());

      // Set horizontal center alignment
      SimpleAttributeSet attrib_set = new SimpleAttributeSet();
      StyleConstants.setAlignment(attrib_set, StyleConstants.ALIGN_CENTER);
      setParagraphAttributes(attrib_set, false);
   }

   /* Centered editor kit for text pane */
   class VerticallyCenteredEditorKit extends StyledEditorKit
   {
      @Override
      public ViewFactory getViewFactory ()
      {
         return new CenteredViewFactory(super.getViewFactory());
      }

      class CenteredViewFactory implements ViewFactory
      {
         private ViewFactory reference_view;
         public CenteredViewFactory (ViewFactory reference_view)
         {
            this.reference_view = reference_view;
         }
         public View create (Element elem)
         {
            if (elem.getName().equals(AbstractDocument.SectionElementName))
               return new CenteredBoxView(elem, View.Y_AXIS);
            else
               return reference_view.create(elem);
         }
      }
   }

   /* Box view with center alignment */
   class CenteredBoxView extends BoxView
   {
      public CenteredBoxView (Element elem, int axis)
      {
         super(elem, axis);
      }

      @Override
      protected void layoutMajorAxis (int target_span, int axis, int[] offsets, int[] spans)
      {
         super.layoutMajorAxis(target_span, axis, offsets, spans);
         int text_block_size = 0;
         int offset = 0;
         for (int i = 0; i < spans.length; i++)
            text_block_size += spans[i];

         offset = (target_span - text_block_size) / 2;
         for (int i = 0; i < offsets.length; i++)
            offsets[i] += offset;
      }
   }
}
