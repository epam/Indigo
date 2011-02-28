package com.ggasoftware.indigo.gui;

import com.ggasoftware.indigo.*;
import javax.swing.*;
import java.util.ArrayList;
import javax.swing.table.*;

public class MolTable extends JPanel
{
   public JTable table;
   public int idx_column_w;
   public static int scroll_w = 20;
   public boolean is_reactions_mode;
   public int cell_w;
   public int cell_h;
   public Indigo indigo;
   public IndigoRenderer indigo_renderer;
   public ArrayList<MolData> mol_datas;
   public IndigoObject mol_iterator;
   public MolSaver mol_saver;
   
   /* Default constructor */
   public MolTable()
   {
      cell_w = 150;
      cell_h = 100;
   }

   public MolTable(Indigo cur_indigo, IndigoRenderer cur_indigo_renderer, MolSaver mol_saver,
           int cur_cell_w, int cur_cell_h, boolean is_reactions)
   {
      int i = 1;
      init(cur_indigo, cur_indigo_renderer, mol_saver, cur_cell_w, cur_cell_h, is_reactions);
   }

   public void init(Indigo cur_indigo, IndigoRenderer cur_indigo_renderer, MolSaver mol_saver,
           int cur_cell_w, int cur_cell_h, boolean is_reactions)
   {
      cell_w = cur_cell_w - scroll_w;
      cell_h = cur_cell_h;
      indigo = cur_indigo;
      indigo_renderer = cur_indigo_renderer;
      this.mol_saver = mol_saver;

      mol_datas = new ArrayList<MolData>();
      MolTableModel mtm = new MolTableModel();
      table = new JTable(mtm);
      TableColumnModel tcm = table.getColumnModel();
      TableColumn tc = tcm.getColumn(0);
      tc.setPreferredWidth(idx_column_w);
      tc.setMaxWidth(idx_column_w);
      tc.setMinWidth(idx_column_w);
      tc.setResizable(true);
      tcm.getColumn(1).setResizable(true);
      table.setRowHeight(cell_h);
      is_reactions_mode = is_reactions;


      table.setDefaultRenderer(MolCell.class, new MolRenderer(indigo, indigo_renderer, this,
              cell_w, cell_h, is_reactions));
      table.setDefaultRenderer(String[].class, new MultiLineCellRenderer(SwingConstants.CENTER,
              SwingConstants.CENTER));

      JScrollPane scrollPane = new JScrollPane(table);
      scrollPane.setViewportView(table);

      GroupLayout gl = new GroupLayout(this);
      setLayout(gl);

      gl.setAutoCreateGaps(true);
      gl.setHorizontalGroup(gl.createParallelGroup(GroupLayout.Alignment.LEADING).
              addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
      gl.setVerticalGroup(gl.createSequentialGroup().
              addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));

      table.addMouseListener(new MolClicker(indigo, indigo_renderer, mol_saver));

      clear();
   }

   public ArrayList<MolData> getMolecules()
   {
      return mol_datas;
   }

   public void setColumnWidth(int column, int width, int width_shift) {
      TableColumnModel tcm = table.getColumnModel();
      TableColumn tc = tcm.getColumn(column);
      tc.setPreferredWidth(width);
      tc.setMaxWidth(width + width_shift);
      tc.setMinWidth(width);
      table.setColumnModel(tcm);
   }

   public void setRowHeight(int height) {
      table.setRowHeight(height);
   }

   public void update() {
      table.updateUI();
      table.revalidate();
   }

   public void setColumnName(int column, String name) {
      table.getColumnModel().getColumn(column).setHeaderValue(name);
      MolTableModel mtm = (MolTableModel) table.getModel();
      mtm.setColumnName(column, name);
      table.setModel(mtm);

      table.tableChanged(null);
      setColumnWidth(0, 40, 300 - cell_w - 10);
      setRowHeight(cell_h);

      table.getTableHeader().resizeAndRepaint();

      update();
   }

   public void setMols(ArrayList<MolData> mols,
           ArrayList<ArrayList<Integer>> indexes1,
           ArrayList<ArrayList<Integer>> indexes2) {
      MolTableModel mtm = (MolTableModel) table.getModel();
      mtm.setMols(mols, indexes1, indexes2, is_reactions_mode);
      mtm.setColumnName(1, mols.size() + " molecules");
      table.setModel(mtm);
      mol_datas.clear();
      mol_datas.addAll(mols);

      int msize = mols.size();
      int digits_count = 0;
      while (msize != 0) {
         digits_count++;
         msize /= 10;
      }

      table.tableChanged(null);
      setColumnWidth(0, (int) (digits_count * table.getFont().getSize2D()), 300 - cell_w - 10);
      setRowHeight(cell_h);

      update();
   }

   public void setMols(ArrayList<MolData> mols) {
      MolTableModel mtm = (MolTableModel) table.getModel();
      mtm.setMols(mols, null, null, is_reactions_mode);
      mtm.setColumnName(1, mols.size() + " molecules");
      table.setModel(mtm);
      mol_datas.clear();
      mol_datas.addAll(mols);

      int msize = mols.size();
      int digits_count = 0;

      while (msize != 0) {
         digits_count++;
         msize /= 10;
      }

      table.tableChanged(null);
      setColumnWidth(0, (int) ((digits_count - 1) * table.getFont().getSize2D()), 300 - cell_w - 10);
      setRowHeight(cell_h);

      update();
   }

   public void clear() {
      MolTableModel mtm = (MolTableModel) table.getModel();
      mtm.clear();
      mtm.setColumnName(1, "0 molecules");
      table.setModel(mtm);

      table.tableChanged(null);

      DefaultTableColumnModel tcm = (DefaultTableColumnModel) table.getColumnModel();
      TableColumn tc = tcm.getColumn(0);
      tc.setPreferredWidth(idx_column_w);
      tc.setMaxWidth(idx_column_w);
      tc.setMinWidth(idx_column_w);
      table.setRowHeight(cell_h);
      update();

      mol_datas.clear();
   }

   private static class MolTableModel extends AbstractTableModel {

      String column_names[] = {"Idx", "Molecules"};
      Object[][] values = {};

      public Object getValueAt(int row, int column) {
         return values[row][column];
      }

      public String getColumnName(int column) {
         return column_names[column];
      }

      public void setColumnName(int column, String name) {
         column_names[column] = name.substring(0);
      }

      public int getColumnCount() {
         return 2;
      }

      public Class getColumnClass(int column) {
         return values[0][column].getClass();
      }

      public int getRowCount() {
         return values.length;
      }

      public void setValueAt(Object value, int row, int column) {
         values[row][column] = value;
         fireTableCellUpdated(row, column);
      }

      public void setMols(ArrayList<MolData> mol_datas,
              ArrayList< ArrayList<Integer> > indexes1,
              ArrayList< ArrayList<Integer> > indexes2, boolean is_rxn) {
         values = new Object[mol_datas.size()][2];
         MolCell mol_images[] = new MolCell[mol_datas.size()];

         for (int i = 0; i < mol_datas.size(); i++)
         {
            mol_images[i] = new MolCell(mol_datas.get(i), is_rxn);

            ArrayList<String> str_array = new ArrayList<String>();
            if (indexes1 == null) {
               str_array.add("" + (i + 1));
            } else {
               String str = "";
               for (int j = 0; j < indexes1.get(i).size(); j++)
               {
                  if (j > 0)
                     str += ",";
                  str += "" + (indexes1.get(i).get(j) + 1);
               }
               str_array.add(str);
            }
            if (indexes2 != null) {
               String str = "";
               for (int j = 0; j < indexes2.get(i).size(); j++)
               {
                  if (j > 0)
                     str += ",";
                  str += "" + (indexes2.get(i).get(j) + 1);
               }
               str_array.add(str);
            }

            String[] tmp_array = new String[str_array.size()];
            setValueAt(str_array.toArray(tmp_array), i, 0);
            setValueAt(mol_images[i], i, getColumnCount() - 1);
         }
      }

      public void clear() {
         values = new Object[0][getColumnCount()];
      }
   }
}
