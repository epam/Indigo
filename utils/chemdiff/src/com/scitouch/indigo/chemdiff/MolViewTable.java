package com.scitouch.indigo.chemdiff;

import com.scitouch.indigo.*;
import java.awt.Color;
import java.awt.Component;
import java.awt.ComponentOrientation;
import java.awt.Font;
import java.awt.Graphics;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import javax.swing.*;
import java.util.ArrayList;
import javax.swing.border.Border;
import javax.swing.border.EmptyBorder;
import javax.swing.table.*;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;


public class MolViewTable extends JPanel
{
   private JTable table;
   private int idx_column_w;
   private static int scroll_w = 20;
   private boolean is_reactions_mode;
   private int cell_w;
   private int cell_h;
   private Indigo indigo;
   private IndigoRenderer indigo_renderer;
   ArrayList<IndigoObject> molecules;
   SdfLoader sdf_loader;
   MainFrame main_frame;
   private int table_idx;

   /* Default constructor */
   public MolViewTable( Indigo cur_indigo, IndigoRenderer cur_indigo_renderer,
                        MainFrame cur_main_frame, int cur_table_idx,
                        int cur_cell_w, int cur_cell_h, boolean is_reactions )
   {
      main_frame = cur_main_frame;
      table_idx = cur_table_idx;
      cell_w = cur_cell_w - scroll_w;
      cell_h = cur_cell_h;
      indigo = cur_indigo;
      indigo_renderer = cur_indigo_renderer;

      sdf_loader = new SdfLoader(indigo, main_frame, table_idx, true);

      molecules = new ArrayList<IndigoObject>();
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
/*
      tcm.getColumn(0).setCellRenderer(new MultiLineCellRenderer(SwingConstants.CENTER,
                                                                       SwingConstants.CENTER));
      tcm.getColumn(1).setCellRenderer(new MolRenderer(indigo, this,
                cell_w - (idx_ccolumn_w + scroll_w), cell_h, is_reactions));
*/
      table.setDefaultRenderer(MolCell.class, new MolRenderer(indigo, indigo_renderer, this,
                cell_w, cell_h, is_reactions));
      table.setDefaultRenderer(String[].class, new MultiLineCellRenderer(SwingConstants.CENTER,
                                                                       SwingConstants.CENTER));
 
      //table.setDefaultRenderer(String.class, new IdxRenderer());

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

      clear();
   }

   public String openSdf( CurDir cur_dir )
   {
      try
       {
          JFileChooser file_chooser = new JFileChooser();
          MolFileFilter mon_ff = new MolFileFilter();
          mon_ff.addExtension("sdf");
          mon_ff.addExtension("sd");
          mon_ff.addExtension("smi");
          file_chooser.setFileFilter(mon_ff);
          file_chooser.setCurrentDirectory(new File(cur_dir.dir_path));
          int ret_val = file_chooser.showOpenDialog(this);
          File choosed_file = file_chooser.getSelectedFile();

          if ((choosed_file == null) || (ret_val != JFileChooser.APPROVE_OPTION))
             return null;

          clear();
          molecules.clear();
          cur_dir.dir_path = choosed_file.getParent();

          main_frame.getLoadProgressBar(table_idx).setMinimum(0);
          main_frame.getLoadProgressBar(table_idx).setMaximum((int)choosed_file.length());

          while ((sdf_loader != null) && (sdf_loader.isActive()))
             sdf_loader.interrupt();

          if (choosed_file.getPath().endsWith(".smi"))
             sdf_loader = new SdfLoader(indigo, main_frame, table_idx, false);
          else
             sdf_loader = new SdfLoader(indigo, main_frame, table_idx, true);

          sdf_loader.setFile(choosed_file);
          sdf_loader.start();

          /*
          int file_pos = 0;
          int old_cnt = 0;
          long old_time = 0;
          for (IndigoObject iterr : indigo.iterateSDFile(choosed_file.getPath()))
          {
             file_pos = indigo.indigoTellSDF(iterr.self);
             try
             {
                molecules.add(iterr.clone());
             }
             catch ( Exception ex )
             {
                molecules.add(null);
             }

             progress_bar.setValue(file_pos);

             progress_bar.updateUI();

             long cur_time = System.currentTimeMillis() / 2000;
             if (old_time != cur_time && old_cnt != molecules.size())
             {
                old_time = cur_time;
                old_cnt = molecules.size();
                System.out.println(molecules.size() + " molecules loaded");
             }
          }

          System.out.println(molecules.size() + " molecules loaded");
          */

          return choosed_file.getPath();
       } catch (Exception ex)
       {
          JOptionPane msg_box = new JOptionPane();
          msg_box.showMessageDialog((JFrame)(getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);

          System.err.println(">>>>" + ex.getMessage() );
          ex.printStackTrace();

          return null;
       }
   }

   public SdfLoader getSdfLoader()
   {
      return sdf_loader;
   }

   public String saveSdf( CurDir cur_dir )
   {
      try
       {
          JFileChooser file_chooser = new JFileChooser();
          MolFileFilter mon_ff = new MolFileFilter();
          mon_ff.addExtension("sdf");
          mon_ff.addExtension("sd");
          file_chooser.setFileFilter(mon_ff);
          file_chooser.setCurrentDirectory(new File(cur_dir.dir_path));
          int ret_val = file_chooser.showSaveDialog(this);
          File choosed_file = file_chooser.getSelectedFile();

          if ((choosed_file == null) || (ret_val != JFileChooser.APPROVE_OPTION))
             return null;
          cur_dir.dir_path = choosed_file.getParent();

          String out_file_path = choosed_file.getPath();
          if (!out_file_path.endsWith(".sdf") && !out_file_path.endsWith(".sd"))
               out_file_path += ".sdf";

          FileWriter fwriter = new FileWriter(out_file_path);
          for (int i = 0; i < molecules.size(); i++)
          {
             fwriter.write(molecules.get(i).molfile());
             String[] orig_id_strs = (String[])table.getValueAt(i, 0);
             fwriter.write("> <original_id>\n" + orig_id_strs[0] + "\n\n");
             fwriter.write("$$$$\n");
          }

          fwriter.close();

          return choosed_file.getPath();
       } catch (Exception ex)
       {
          JOptionPane msg_box = new JOptionPane();
          msg_box.showMessageDialog((JFrame)(getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);

          return null;
       }
   }


   public void setColumnWidth( int column, int width, int width_shift )
   {
      TableColumnModel tcm = table.getColumnModel();
      TableColumn tc = tcm.getColumn(column);
      tc.setPreferredWidth(width);
      tc.setMaxWidth(width + width_shift);
      tc.setMinWidth(width);
      table.setColumnModel(tcm);
   }

   public void setRowHeight( int height )
   {
      table.setRowHeight(height);
   }

   public void update()
   {
      table.updateUI();
      table.revalidate();
   }

   public void setColumnName( int column, String name )
   {
      table.getColumnModel().getColumn(column).setHeaderValue(name);
      MolTableModel mtm = (MolTableModel)table.getModel();
      mtm.setColumnName(column, name);
      table.setModel(mtm);

      table.tableChanged(null);
      setColumnWidth(0, 40, 300 - cell_w - 10);
      setRowHeight(cell_h);

      table.getTableHeader().resizeAndRepaint();

      update();
   }

   public void setMols( ArrayList<IndigoObject> mols,
                        ArrayList<Integer> indexes1,
                        ArrayList<Integer> indexes2 )
   {
      MolTableModel mtm = (MolTableModel)table.getModel();
      mtm.setMols(mols, indexes1, indexes2, is_reactions_mode);
      mtm.setColumnName(1, mols.size() + " molecules");
      table.setModel(mtm);
      molecules.addAll(mols);

      int msize = mols.size();
      int digits_count = 0;
      while (msize != 0)
      {
         digits_count++;
         msize /= 10;
      }

      table.tableChanged(null);
      setColumnWidth(0, (int)(digits_count * table.getFont().getSize2D()), 300 - cell_w - 10);
      setRowHeight(cell_h);

      update();
   }

   public void setMols( ArrayList<IndigoObject> mols )
   {
      MolTableModel mtm = (MolTableModel)table.getModel();
      mtm.setMols(mols, null, null, is_reactions_mode);
      mtm.setColumnName(1, mols.size() + " molecules");
      table.setModel(mtm);
      molecules.addAll(mols);

      int msize = mols.size();
      int digits_count = 0;

      while (msize != 0)
      {
         digits_count++;
         msize /= 10;
      }
      
      table.tableChanged(null);
      setColumnWidth(0, (int)((digits_count - 1) * table.getFont().getSize2D()), 300 - cell_w - 10);
      setRowHeight(cell_h);

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
      tc.setPreferredWidth(idx_column_w);
      tc.setMaxWidth(idx_column_w);
      tc.setMinWidth(idx_column_w);
      table.setRowHeight(cell_h);
      update();
   }


   public class IdxRenderer extends JTextPane implements TableCellRenderer
   {
      public IdxRenderer()
      {
         //setLineWrap(true);
         //setWrapStyleWord(true);
         StyledDocument styled_doc = getStyledDocument();
         SimpleAttributeSet center_as = new SimpleAttributeSet();
         StyleConstants.setAlignment(center_as, StyleConstants.ALIGN_CENTER);
         styled_doc.setParagraphAttributes(0, styled_doc.getLength(), center_as, false);
         setOpaque(true);
      }

      public Component getTableCellRendererComponent(JTable table, Object value,
                   boolean isSelected, boolean hasFocus, int row, int column)
      {
         String str = (String)value;

         setText(str);
         return this;
      }
   }

   class MultiLineCellRenderer extends JPanel implements TableCellRenderer {
  public MultiLineCellRenderer(int horizontalAlignment, int verticalAlignment) {
    this.horizontalAlignment = horizontalAlignment;
    this.verticalAlignment = verticalAlignment;
    switch (horizontalAlignment) {
    case SwingConstants.LEFT:
      alignmentX = (float) 0.0;
      break;

    case SwingConstants.CENTER:
      alignmentX = (float) 0.5;
      break;

    case SwingConstants.RIGHT:
      alignmentX = (float) 1.0;
      break;

    default:
      throw new IllegalArgumentException(
          "Illegal horizontal alignment value");
    }

    setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
    setOpaque(true);
    setBorder(border);

    background = null;
    foreground = null;
  }

  public void setForeground(Color foreground) {
    super.setForeground(foreground);
    Component[] comps = this.getComponents();
    int ncomp = comps.length;
    for (int i = 0; i < ncomp; i++) {
      Component comp = comps[i];
      if (comp instanceof JLabel) {
        comp.setForeground(foreground);
      }
    }
  }

  public void setBackground(Color background) {
    this.background = background;
    super.setBackground(background);
  }

  public void setFont(Font font) {
    this.font = font;
  }

  // Implementation of TableCellRenderer interface
  public Component getTableCellRendererComponent(JTable table, Object value,
      boolean isSelected, boolean hasFocus, int row, int column) {
    removeAll();
    invalidate();

    if (value == null || table == null) {
      // Do nothing if no value
      return this;
    }

    Color cellForeground;
    Color cellBackground;

    // Set the foreground and background colors
    // from the table if they are not set
    cellForeground = (foreground == null ? table.getForeground()
        : foreground);
    cellBackground = (background == null ? table.getBackground()
        : background);

    // Handle selection and focus colors
    if (isSelected == true) {
      cellForeground = table.getSelectionForeground();
      cellBackground = table.getSelectionBackground();
    }

    if (hasFocus == true) {
      setBorder(UIManager.getBorder("Table.focusCellHighlightBorder"));
      if (table.isCellEditable(row, column)) {
        cellForeground = UIManager
            .getColor("Table.focusCellForeground");
        cellBackground = UIManager
            .getColor("Table.focusCellBackground");
      }
    } else {
      setBorder(border);
    }

    super.setForeground(cellForeground);
    super.setBackground(cellBackground);

    // Default the font from the table
    if (font == null) {
      font = table.getFont();
    }

    if (verticalAlignment != SwingConstants.TOP) {
      add(Box.createVerticalGlue());
    }

    Object[] values;
    int length;
    if (value instanceof Object[]) {
      // Input is an array - use it
      values = (Object[]) value;
    } else {
      // Not an array - turn it into one
      values = new Object[1];
      values[0] = value;
    }
    length = values.length;

    // Configure each row of the cell using
    // a separate JLabel. If a given row is
    // a JComponent, add it directly..
    for (int i = 0; i < length; i++) {
      Object thisRow = values[i];

      if (thisRow instanceof JComponent) {
        add((JComponent) thisRow);
      } else {
        JLabel l = new JLabel();
        setValue(l, thisRow, i, cellForeground);
        add(l);
      }
    }

    if (verticalAlignment != SwingConstants.BOTTOM) {
      add(Box.createVerticalGlue());
    }
    return this;
  }

  // Configures a label for one line of the cell.
  // This can be overridden by derived classes
  protected void setValue(JLabel l, Object value, int lineNumber,
      Color cellForeground) {
    if (value != null && value instanceof Icon) {
      l.setIcon((Icon) value);
    } else {
      l.setText(value == null ? "" : value.toString());
    }
    l.setHorizontalAlignment(horizontalAlignment);
    l.setAlignmentX(alignmentX);
    l.setOpaque(false);
    l.setForeground(cellForeground);
    l.setFont(font);
  }

  protected int verticalAlignment;

  protected int horizontalAlignment;

  protected float alignmentX;

  // These attributes may be explicitly set
  // They are defaulted to the colors and attributes
  // of the table

  protected Color foreground;

  protected Color background;

  protected Font font;

  protected Border border = new EmptyBorder(1, 2, 1, 2);
}


   private static class MolTableModel extends AbstractTableModel
   {
      String column_names[] = {"Idx", "Molecules"};
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

      public void setMols( ArrayList<IndigoObject> mols,
                           ArrayList<Integer> indexes1,
                           ArrayList<Integer> indexes2, boolean is_rxn )
      {
         values = new Object[mols.size()][2];
         MolCell mol_images[] = new MolCell[mols.size()];

         for (int i = 0; i < mols.size(); i++)
         {
            mol_images[i] = new MolCell(mols.get(i), is_rxn);

            ArrayList<String> str_array = new ArrayList<String>();
            if (indexes1 == null)
               str_array.add("" + (i + 1));
            else
               str_array.add("" + (indexes1.get(i) + 1));
            if (indexes2 != null)
               str_array.add("" + (indexes2.get(i) + 1));

            String[] tmp_array = new String[str_array.size()];
            setValueAt(str_array.toArray(tmp_array), i, 0);
            setValueAt(mol_images[i], i, 1);
         }
      }

      public void clear()
      {
         values = new Object[0][2];
      }
   }
}
