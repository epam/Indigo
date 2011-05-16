package com.ggasoftware.indigo.controls;

import java.awt.Component;
import javax.swing.AbstractCellEditor;
import javax.swing.JTable;
import javax.swing.UIManager;
import javax.swing.table.TableCellEditor;

public class MultilineCenteredTableCellEditor extends AbstractCellEditor implements TableCellEditor
{
   private CenteredTextPane component = new CenteredTextPane();
   private boolean editable;

   public MultilineCenteredTableCellEditor (boolean editable)
   {
      this.editable = editable;
   }
   
   public Component getTableCellEditorComponent (JTable table, Object value,
           boolean isSelected, int rowIndex, int vColIndex)
   {
      component.setEditable(editable);
      component.setText((String)value);
      component.setBorder(UIManager.getBorder("Table.focusCellHighlightBorder"));
      return component;
   }

   public Object getCellEditorValue ()
   {
      return component.getText();
   }
}
