package com.ggasoftware.indigo.controls;

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JTable;

public class TableCellMouseHandler extends MouseAdapter
{
   private Object active_object = null;
   private int active_object_row = -1;
   private int target_column_idx;
   private Announcer<TableCellMouseListener> cell_event_listeners = 
           Announcer.to(TableCellMouseListener.class);

   public TableCellMouseHandler (int target_column_idx)
   {
      this.target_column_idx = target_column_idx;
   }

   public void setTargetColumn (int target_column_idx)
   {
      this.target_column_idx = target_column_idx;
   }
   
   public int getTargetColumn ()
   {
      return target_column_idx;
   }
   
   public Object getActiveObject ()
   {
      return active_object;
   }
   
   public int getActiveObjectRow ()
   {
      return active_object_row;
   }

   @Override
   public void mousePressed (MouseEvent e)
   {
      maybeShowPopup(e);
   }

   @Override
   public void mouseReleased (MouseEvent e)
   {
      maybeShowPopup(e);
   }
   
   private void maybeShowPopup (MouseEvent me)
   {
      if (me.isPopupTrigger() && validateMouseEventData(me))
         onShowPopup(me);
   }
   
   @Override
   public void mouseClicked (MouseEvent me)
   {
      if (me.getButton() == MouseEvent.BUTTON1)
      {
         if (me.getClickCount() == 2 && validateMouseEventData(me))
            onDoubleLeftClick(me);
         return;
      }
   }

   private boolean validateMouseEventData (MouseEvent me)
   {
      JTable table = (JTable)me.getSource();

      int col = table.columnAtPoint(me.getPoint());
      int row = table.rowAtPoint(me.getPoint());

      if (col != target_column_idx)
         return false;

      active_object_row = row;
      active_object = table.getValueAt(row, col);
      return true;
   }

   private TableCellMouseEvent createEvent (MouseEvent me)
   {
      return new TableCellMouseEvent(me.getSource(), 
              active_object_row, target_column_idx, me);
   }

   private void onShowPopup (MouseEvent me)
   {
      cell_event_listeners.announce().cellShowPopupMenu(createEvent(me));
   }

   private void onDoubleLeftClick (MouseEvent me)
   {
      cell_event_listeners.announce().cellMouseDoubleClick(createEvent(me));
   }
   
   public void addTableCellMouseListener (TableCellMouseListener listener)
   {
      cell_event_listeners.addListener(listener);
   }

   public void removeTableCellMouseListener (TableCellMouseListener listener)
   {
      cell_event_listeners.removeListener(listener);
   }
}
