package com.ggasoftware.indigo.gui;

public class ProgressEvent
{
   public int table_idx;
   public int progress;

   public ProgressEvent (int table_idx, int progress)
   {
      this.table_idx = table_idx;
      this.progress = progress;
   }
}
