package com.ggasoftware.indigo.gui;

import com.ggasoftware.indigo.gui.MolEvent;

public class ProgressEvent extends MolEvent
{
   public int table_idx;
   public int progress;

   public ProgressEvent( int table_idx, int progress )
   {
      this.table_idx = table_idx;
      this.progress = progress;
   }
}
