package com.ggasoftware.indigo.controls;

import java.awt.Component;
import javax.swing.JTable;
import javax.swing.UIManager;
import javax.swing.border.EmptyBorder;
import javax.swing.table.TableCellRenderer;

public class MultilineCenteredTableCellRenderer extends CenteredTextPane
        implements TableCellRenderer
{
   public MultilineCenteredTableCellRenderer ()
   {
      setOpaque(true);
   }

   public Component getTableCellRendererComponent (JTable table,
           Object value, boolean isSelected, boolean hasFocus, int row,
           int column)
   {
      if (isSelected)
      {
         setForeground(table.getSelectionForeground());
         setBackground(table.getSelectionBackground());
      }
      else
      {
         setForeground(table.getForeground());
         setBackground(table.getBackground());
      }
      if (hasFocus)
      {
         setBorder(UIManager.getBorder("Table.focusCellHighlightBorder"));
         if (table.isCellEditable(row, column))
         {
            setForeground(UIManager.getColor("Table.focusCellForeground"));
            setBackground(UIManager.getColor("Table.focusCellBackground"));
         }
      }
      else
         setBorder(new EmptyBorder(1, 2, 1, 2));
      
      setFont(table.getFont());
      setText(value.toString());

      return this;
   }
}