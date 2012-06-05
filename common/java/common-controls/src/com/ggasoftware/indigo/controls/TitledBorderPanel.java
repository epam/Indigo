package com.ggasoftware.indigo.controls;

import javax.swing.JPanel;

public class TitledBorderPanel extends JPanel
{
   private String _title = null;
   private String _subtitle = null;
   
   private String getFullTitle ()
   {
      if (_title == null)
         return null;
      if (_subtitle == null)
         return _title;
      return _title + _subtitle;
   }
   
   private void validateTitle ()
   {
      if (getFullTitle() == null)
         setBorder(javax.swing.BorderFactory.createEmptyBorder());
      else
         setBorder(javax.swing.BorderFactory.createTitledBorder(getFullTitle()));
   }
   
   public void setTitle (String title)
   {
      _title = title;
      validateTitle();
   }
   
   public String getTitle ()
   {
      return _title;
   }
   
   public void setSubtitle (String subtitle)
   {
      _subtitle = subtitle;
      validateTitle();
   }

   public String getSubtitle ()
   {
      return _subtitle;
   }
}
