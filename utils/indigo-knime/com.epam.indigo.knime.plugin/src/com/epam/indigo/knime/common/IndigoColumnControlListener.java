package com.epam.indigo.knime.common;

import javax.swing.JCheckBox;
import javax.swing.JTextField;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.knime.core.node.util.ColumnSelectionComboxBox;

public class IndigoColumnControlListener implements ChangeListener {
   
   private final JTextField txtNewColName;
   private final String suffix;
   private final ColumnSelectionComboxBox cmbIndigoColumn;
   
   public IndigoColumnControlListener(ColumnSelectionComboxBox indigoColumn, JTextField newColName, String suffix)
   {
      txtNewColName = newColName;
      this.suffix = suffix; 
      cmbIndigoColumn = indigoColumn;
   }
   
   @Override
   public void stateChanged(ChangeEvent arg0) {
      final JCheckBox appendColumn = (JCheckBox)arg0.getSource();
      
      if (appendColumn.isSelected()) {
         txtNewColName.setEnabled(true);
         if ("".equals(txtNewColName.getText())) {
            txtNewColName.setText(cmbIndigoColumn.getSelectedColumn() + " " + suffix);
         }
      } else {
         txtNewColName.setEnabled(false);
      }
   }
}
