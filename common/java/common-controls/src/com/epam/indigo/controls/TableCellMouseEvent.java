package com.epam.indigo.controls;

import java.awt.event.MouseEvent;
import java.util.EventObject;

public class TableCellMouseEvent extends EventObject
{
   public TableCellMouseEvent(Object source, int row, int column, MouseEvent mouse_event)
   {
      super(source);
      this.row = row;
      this.column = column;
      this.mouse_event = mouse_event;
   }
   public int row, column;
   public MouseEvent mouse_event;
}
