package com.ggasoftware.indigo.legio;

import com.ggasoftware.indigo.*;
import javax.swing.*;
import java.util.ArrayList;
import javax.swing.table.*;


public class MolViewTable extends JPanel
{
   private JTable table;
   private static int idx_ccolumn_w = 30;
   private static int scroll_w = 26;
   private int cell_w;
   private int cell_h;
   private boolean is_reactions_mode;
   private Indigo indigo;
   private IndigoRenderer indigo_renderer;


   /* Default constructor */
   public MolViewTable( Indigo cur_indigo, IndigoRenderer cur_indigo_renderer,
                        int new_cell_w, int new_cell_h, boolean is_reactions )
   {
      indigo = cur_indigo;
      indigo_renderer = cur_indigo_renderer;
      MolTableModel mtm = new MolTableModel();
      table = new JTable(mtm);
      is_reactions_mode = is_reactions;

      cell_w = new_cell_w;
      cell_h = new_cell_h;

      /////////
      DefaultTableColumnModel tcm = (DefaultTableColumnModel)table.getColumnModel();
      TableColumn tc = tcm.getColumn(0);
      tc.setPreferredWidth(idx_ccolumn_w);
      tc.setMaxWidth(idx_ccolumn_w);
      tc.setMinWidth(idx_ccolumn_w);
      table.setRowHeight(cell_h);
      /////////

      table.setDefaultRenderer(MolCell.class, new MolRenderer(indigo, indigo_renderer,
              new_cell_w - (idx_ccolumn_w + scroll_w), new_cell_h, is_reactions));
      table.setDefaultRenderer(int.class, new IdxRenderer());

      JScrollPane scrollPane = new JScrollPane(table);
      scrollPane.setViewportView(table);

      GroupLayout gl = new GroupLayout(this);
      setLayout(gl);

      gl.setAutoCreateGaps(true);
      gl.setHorizontalGroup(gl.createParallelGroup(GroupLayout.Alignment.LEADING).
               addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
      gl.setVerticalGroup(gl.createSequentialGroup().
              addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
   
      table.addMouseListener(new MolClicker(indigo, indigo_renderer));
   }

   public void setColumnWidth( int column, int width )
   {
      TableColumnModel tcm = table.getColumnModel();
      TableColumn tc = tcm.getColumn(column);
      tc.setPreferredWidth(width);
      tc.setMaxWidth(width);
      tc.setMinWidth(width);
      table.setColumnModel(tcm);
   }

   public void update()
   {
      table.revalidate();
      table.repaint();
   }

   public void setMols( ArrayList<IndigoObject> mols )
   {
      MolTableModel mtm = (MolTableModel)table.getModel();
      mtm.setMols(mols, is_reactions_mode);
      mtm.setColumnName(1, mols.size() + " molecules");
      table.setModel(mtm);

      table.tableChanged(null);

      DefaultTableColumnModel tcm = (DefaultTableColumnModel)table.getColumnModel();
      TableColumn tc = tcm.getColumn(0);
      tc.setPreferredWidth(idx_ccolumn_w);
      tc.setMaxWidth(idx_ccolumn_w);
      tc.setMinWidth(idx_ccolumn_w);
      table.setRowHeight(cell_h);
      update();
   }
   
   public void clear()
   {
      MolTableModel mtm = (MolTableModel)table.getModel();
      mtm.clear();
      mtm.setColumnName(1, "0 molecules");
      table.setModel(mtm);

      table.tableChanged(null);

      DefaultTableColumnModel tcm = (DefaultTableColumnModel)table.getColumnModel();
      TableColumn tc = tcm.getColumn(0);
      tc.setPreferredWidth(idx_ccolumn_w);
      tc.setMaxWidth(idx_ccolumn_w);
      tc.setMinWidth(idx_ccolumn_w);
      table.setRowHeight(cell_h);
      update();
   }


   private static class IdxRenderer extends DefaultTableCellRenderer
   {
      public IdxRenderer() {
          setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
      }
   }

   private static class MolTableModel extends AbstractTableModel
   {
      String column_names[] = {"idx", "molecules"};
      Object[][] values = {};

      public Object getValueAt( int row, int column )
      {
         return values[row][column];
      }

      public String getColumnName( int column )
      {
         return column_names[column];
      }

      public void setColumnName( int column, String name )
      {
         column_names[column] = name.substring(0);
      }

      public int getColumnCount()
      {
         return 2;
      }

      public Class getColumnClass( int column )
      {
         return values[0][column].getClass();
      }

      public int getRowCount()
      {
         return values.length;
      }

      public void setValueAt( Object value, int row, int column )
      {
         values[row][column] = value;
         fireTableCellUpdated(row, column);
      }

      public void setMols( ArrayList<IndigoObject> mols, boolean is_rxn )
      {
         values = new Object[mols.size()][2];
         MolCell mol_images[] = new MolCell[mols.size()];

         for (int i = 0; i < mols.size(); i++)
         {
            mol_images[i] = new MolCell(mols.get(i), is_rxn);
            setValueAt((i + 1), i, 0);
            setValueAt(mol_images[i], i, 1);
         }
      }

      public void clear()
      {
         values = new Object[0][2];
      }
   }
}
