/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.legio;

import com.ggasoftware.indigo.IndigoObject;
import java.util.ArrayList;
import javax.swing.table.DefaultTableModel;

public class MolTableModel extends DefaultTableModel
{
   private boolean _is_reactions;

   public MolTableModel(boolean is_reactions)
   {
      this._is_reactions = is_reactions;
      Object[][] objects = new Object[][] {};
      String[] identifiers = (is_reactions ? new String[] {"Id", "Reactions"} :
                                             new String[] {"Id", "Molecules"});
      setDataVector(objects, identifiers);
   }

   public String getColumnName(int idx_col)
   {
      if (idx_col > 1 || idx_col < 0)
         return null;

      if (idx_col == 0)
         return "Id";
      else if (_is_reactions)
         return "Reactions";
      else
         return "Molecules";
   }

   public boolean isCellEditable(int rowIndex, int columnIndex) {
      return false;
   }

   public void addMols(ArrayList<IndigoObject> mol_datas) {
      int old_row_count = getRowCount();
      for (int i = 0; i < mol_datas.size(); i++)
      {
         ArrayList<Object> objects = new ArrayList<Object>();

         objects.add(i + old_row_count);
         objects.add(new RenderableIndigoObject(mol_datas.get(i)));
         addRow(objects.toArray());
      }
   }

   public void setMols(ArrayList<IndigoObject> mol_datas) {
      clear();
      addMols(mol_datas);
   }

   public int getMolsCount() {
      return getRowCount();
   }

   public void clear() {
      while (getRowCount() != 0)
         removeRow(0);
   }
}
