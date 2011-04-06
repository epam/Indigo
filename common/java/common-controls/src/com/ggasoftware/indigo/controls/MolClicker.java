package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.*;
import javax.swing.table.DefaultTableModel;

public class MolClicker extends MouseAdapter
{
   private Indigo _indigo;
   private IndigoRenderer _indigo_renderer;
   private RenderableObject _mol;
   private JPopupMenu _popup_menu;
   private boolean _is_reaction;
   MolSaver mol_saver;

   public MolClicker(Indigo cur_indigo, IndigoRenderer cur_indigo_renderer, 
                     JTable table, boolean is_reaction) {
      _indigo = cur_indigo;
      _indigo_renderer = cur_indigo_renderer;
      _is_reaction = is_reaction;
      _mol = null;
      mol_saver = new MolSaver(_indigo);


      _indigo.setOption("render-comment-font-size", "14");

      _popup_menu = new JPopupMenu();
      JMenuItem show_mi = new JMenuItem("Show in other window");
      show_mi.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            _showCell();
         }
      });
      _popup_menu.add(show_mi);
   }

   @Override
   public void mouseClicked(MouseEvent me) {
      if (me.getButton() == MouseEvent.BUTTON3)
      {
         fireSingleRightClick(me);
         return;
      }
      if (me.getButton() == MouseEvent.BUTTON1)
      {
         if (me.getClickCount() == 2)
            fireDoubleLeftClick(me);
         return;
      }
   }

   private void _showCell()
   {
      CellFrame cell_frame = new CellFrame();
      cell_frame.init(_indigo, _indigo_renderer, _mol, _is_reaction);
   }

   protected void fireSingleRightClick(MouseEvent me)
   {
      JTable table = (JTable) me.getSource();

      int col = table.columnAtPoint(me.getPoint());
      int row = table.rowAtPoint(me.getPoint());

      if (table.getColumnName(col).contains("Id")) {
         return;
      }

      _mol = (RenderableObject)table.getValueAt(row, col);

      if (_mol.getObject() == null)
         return;

      _popup_menu.show(table, me.getX(), me.getY());
   }

   protected void fireDoubleLeftClick(MouseEvent me) {
      JTable table = (JTable) me.getSource();

      int col = table.columnAtPoint(me.getPoint());
      int row = table.rowAtPoint(me.getPoint());

      if (table.getColumnName(col).contains("Id")) {
         return;
      }

      _mol = (RenderableObject)table.getValueAt(row, col);

      if (_mol.getObject() == null)
         return;

      _showCell();
   }
}
