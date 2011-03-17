package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.*;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.Toolkit;
import javax.swing.*;
import javax.swing.table.DefaultTableColumnModel;

public class MolClicker extends MouseAdapter {

   private int doubleClickDelay = 300;
   private Timer timer;
   private Indigo indigo;
   private IndigoRenderer indigo_renderer;
   private RenderableMolData mol;
   MolSaver mol_saver;

   public MolClicker(Indigo cur_indigo, IndigoRenderer cur_indigo_renderer, MolSaver mol_saver) {
      indigo = cur_indigo;
      indigo_renderer = cur_indigo_renderer;
      this.mol_saver = mol_saver;
      mol = null;

      indigo.setOption("render-comment-font-size", "14");
      
      ActionListener actionListener = new ActionListener()
        {

         @Override
         public void actionPerformed(ActionEvent e) {
            timer.stop();
         }
      };

      timer = new Timer(doubleClickDelay, actionListener);
      timer.setRepeats(false);
   }

   @Override
   public void mouseClicked(MouseEvent e) {
      ActionEvent ev = new ActionEvent(e.getSource(), e.getID(), e.paramString());
      if (timer.isRunning()) {
         timer.stop();
         fireSingleClick(ev);
         fireDoubleClick(ev);
      } else {
         timer.start();
         fireSingleClick(ev);
      }
   }

   protected void fireSingleClick(ActionEvent e) {
   }

   protected void fireDoubleClick(ActionEvent e) {
      JTable table = (JTable) e.getSource();

      int col = table.getSelectedColumn();
      int row = table.getSelectedRow();

      if (col == 0) {
         return;
      }

      mol = (RenderableMolData)table.getValueAt(row, col);

      if (mol.mol_iterator == null)
      {
         return;
      }

      JFrame cell_frame = new JFrame();
      cell_frame.setSize(new Dimension(510, 540));
      JMenuBar cell_menu_bar = new JMenuBar();
      JMenu menu_file = new JMenu("File");
      JMenuItem menu_save_item = new JMenuItem("Save");

      //menu_save_item.setText("Save");
      menu_save_item.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            saveCellActionPerformed(evt);
         }
      });

      menu_file.add(menu_save_item);
      cell_menu_bar.add(menu_file);
      cell_frame.setJMenuBar(cell_menu_bar);


      MolViewPanel mol_view = new MolViewPanel(indigo, indigo_renderer);
      mol_view.setImageSize(500, 500);

      IndigoObject molecule = mol.getObject().clone();

      molecule.layout();

      mol_view.setMol(molecule);

      String mol_name = molecule.name();

      if (mol_name == null || mol_name.length() == 0)
      {
         try {
            mol_name = molecule.canonicalSmiles();
         } catch (Exception ex) {
            mol_name = null;
         }
      }

      cell_frame.setTitle(mol_name);

      GroupLayout gl1 = new GroupLayout(cell_frame);

      gl1.setAutoCreateGaps(true);
      gl1.setHorizontalGroup(gl1.createParallelGroup(GroupLayout.Alignment.LEADING).
              addComponent(mol_view, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
      gl1.setVerticalGroup(gl1.createSequentialGroup().
              addComponent(mol_view, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));

      // Set window position in the middle
      Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
      Dimension windowSize = cell_frame.getSize();

      int windowX = Math.max(0, (screenSize.width - windowSize.width) / 2);
      int windowY = Math.max(0, (screenSize.height - windowSize.height) / 2);

      cell_frame.setLocation(windowX, windowY);

      cell_frame.setVisible(true);
   }

   private void saveCellActionPerformed( ActionEvent evt )
   {
      mol_saver.saveMol(mol);
   }
}
