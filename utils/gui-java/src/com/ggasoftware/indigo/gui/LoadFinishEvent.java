package com.ggasoftware.indigo.gui;

import com.ggasoftware.indigo.gui.MolEvent;

public class LoadFinishEvent extends MolEvent
{
   public int table_idx;

   public LoadFinishEvent( int table_idx )
   {
      this.table_idx = table_idx;
   }
}
