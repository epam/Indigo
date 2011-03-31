/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.chemdiff;

import java.util.ArrayList;
import javax.swing.table.DefaultTableModel;

public class MolTableModel extends DefaultTableModel
{
   private int _idx_column_count;

   public MolTableModel(int idx_column_count)
   {
      this._idx_column_count = idx_column_count;
      Object[][] objects = new Object[][] {};
      String[] identifiers = (idx_column_count == 1 ? new String[] {"Id", "Molecules"} :
                                                      new String[] {"Id1", "Id2", "Molecules"});
      setDataVector(objects, identifiers);
   }

   public String getIdColumnName(int idx_col)
   {
      if (idx_col >= _idx_column_count)
         return null;

      if (_idx_column_count == 1)
         return "Id";
      else
         return "Id" + (idx_col + 1);
   }

   public boolean isCellEditable(int rowIndex, int columnIndex) {
      return false;
   }

   public void setMols(ArrayList<RenderableMolData> mol_datas,
              ArrayList< ArrayList<Integer> > indexes1,
              ArrayList< ArrayList<Integer> > indexes2) {
      while (getRowCount() != 0)
         removeRow(0);

      int i = 0;
      while (getRowCount() < mol_datas.size())
      {
         ArrayList<Object> objects = new ArrayList<Object>();

         for (int j = 0; j < _idx_column_count; j++)
         {
            ArrayList< ArrayList<Integer> > indexes = (j == 0 ? indexes1 : indexes2);
            String[] idx_strings = (indexes != null ? Utils.makeIdxString(indexes.get(i)) :
                                               new String[] {"" + i});
            objects.add(idx_strings);
         }
         objects.add(new RenderableMolData(mol_datas.get(i++)));
         addRow(objects.toArray());
      }
   }
}
