package com.gga.indigo.legio;

import javax.swing.*;
import javax.swing.GroupLayout.Group;
import java.io.*;
import java.util.ArrayList;
import com.gga.indigo.Indigo;
import com.gga.indigo.IndigoObject;
import com.gga.indigo.IndigoRenderer;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class MainFrame extends javax.swing.JFrame
{
   ArrayList<MonomerPanel> mon_panels;
   MolViewTable products_table;
   MolViewPanel rct_view;
   JButton save_products_button;
   JButton save_reactions_button;
   int reactants_count;
   LegioData legio;
   CurDir cur_dir;
   Indigo indigo;
   IndigoRenderer indigo_renderer;

   /* Creates new form MainFrame */
   public MainFrame() {
      indigo = new Indigo();
      indigo_renderer = new IndigoRenderer(indigo);
      mon_panels = new ArrayList<MonomerPanel>();
      rct_view = new MolViewPanel(indigo, indigo_renderer);
      legio = new LegioData(indigo);
      cur_dir = new CurDir();
      
      indigo.setOption("filename-encoding", "UTF-8");
      indigo.setOption("render-margins", "5,2");
      indigo.setOption("render-comment-margins", "5,2");
      indigo.setOption("molfile-saving-mode", "3000");

      initComponents();
      save_products_button = new JButton();
      save_products_button.setText("Save products");
      save_products_button.setEnabled(false);
      save_products_button.addActionListener(new SaveProductsEventListener());
      save_reactions_button = new JButton();
      save_reactions_button.setText("Save reactions");
      save_reactions_button.setEnabled(false);
      save_reactions_button.addActionListener(new SaveReactionsEventListener());
      products_table = new MolViewTable(indigo, indigo_renderer, out_tab.getSize().width - 30, 300, true);
      jLabel1.setEnabled(false);
      jTextField1.setEnabled(false);
      jLabel2.setEnabled(false);
      jTextField2.setEnabled(false);
      react_button.setEnabled(false);
      is_multistep_reactions_check.setEnabled(false);
      is_one_tube_check.setEnabled(false);
      is_self_react_check.setEnabled(false);
      reaction_path_label.setEditable(false);

      GroupLayout gl = new GroupLayout(out_tab);
      out_tab.setLayout(gl);

      gl.setAutoCreateGaps(true);
      gl.setHorizontalGroup(gl.createParallelGroup().
              addComponent(products_table, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE).
              addGroup(gl.createSequentialGroup().
              addComponent(save_products_button, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, 150).
              addComponent(save_reactions_button, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, 150)));
      gl.setVerticalGroup(gl.createSequentialGroup().
              addComponent(products_table, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE).
              addGroup(gl.createParallelGroup().
              addComponent(save_products_button, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, 26).
              addComponent(save_reactions_button, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, 26)));

      setTitle("Legio");
   }

   /** This method is called from within the constructor to
    * initialize the form.
    */
   @SuppressWarnings("unchecked")
   // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
   private void initComponents() {

      jTabbedPane1 = new javax.swing.JTabbedPane();
      in_tab = new javax.swing.JPanel();
      jSplitPane1 = new javax.swing.JSplitPane();
      rct_part = new javax.swing.JPanel();
      reaction_label = new javax.swing.JLabel();
      reaction_button = new javax.swing.JButton();
      rct_panel = new javax.swing.JPanel();
      reaction_path_label = new javax.swing.JTextField();
      mons_part = new javax.swing.JPanel();
      mons_label = new javax.swing.JLabel();
      mons_scroll_panel = new javax.swing.JScrollPane();
      mons_container = new javax.swing.JPanel();
      jPanel1 = new javax.swing.JPanel();
      is_multistep_reactions_check = new java.awt.Checkbox();
      react_button = new javax.swing.JButton();
      jTextField1 = new javax.swing.JTextField();
      jLabel1 = new javax.swing.JLabel();
      jLabel2 = new javax.swing.JLabel();
      jTextField2 = new javax.swing.JTextField();
      is_one_tube_check = new java.awt.Checkbox();
      is_self_react_check = new java.awt.Checkbox();
      out_tab = new javax.swing.JPanel();

      setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

      jSplitPane1.setDividerLocation(300);
      jSplitPane1.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);

      rct_part.setBorder(javax.swing.BorderFactory.createEmptyBorder(5, 5, 5, 5));

      reaction_label.setText("Reaction:");

      reaction_button.setText("Open");
      reaction_button.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            reaction_buttonActionPerformed(evt);
         }
      });

      rct_panel.setBackground(new java.awt.Color(255, 255, 255));

      javax.swing.GroupLayout rct_panelLayout = new javax.swing.GroupLayout(rct_panel);
      rct_panel.setLayout(rct_panelLayout);
      rct_panelLayout.setHorizontalGroup(
         rct_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 914, Short.MAX_VALUE)
      );
      rct_panelLayout.setVerticalGroup(
         rct_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 260, Short.MAX_VALUE)
      );

      javax.swing.GroupLayout rct_partLayout = new javax.swing.GroupLayout(rct_part);
      rct_part.setLayout(rct_partLayout);
      rct_partLayout.setHorizontalGroup(
         rct_partLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(rct_partLayout.createSequentialGroup()
            .addContainerGap()
            .addComponent(reaction_label)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
            .addComponent(reaction_button)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(reaction_path_label, javax.swing.GroupLayout.PREFERRED_SIZE, 773, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
         .addComponent(rct_panel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
      );
      rct_partLayout.setVerticalGroup(
         rct_partLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(rct_partLayout.createSequentialGroup()
            .addGroup(rct_partLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
               .addComponent(reaction_path_label, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
               .addGroup(rct_partLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                  .addComponent(reaction_label)
                  .addComponent(reaction_button, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(rct_panel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
      );

      jSplitPane1.setTopComponent(rct_part);

      mons_label.setText("Monomers:");

      mons_container.setLayout(new javax.swing.BoxLayout(mons_container, javax.swing.BoxLayout.LINE_AXIS));
      mons_scroll_panel.setViewportView(mons_container);

      javax.swing.GroupLayout mons_partLayout = new javax.swing.GroupLayout(mons_part);
      mons_part.setLayout(mons_partLayout);
      mons_partLayout.setHorizontalGroup(
         mons_partLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(mons_scroll_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 924, Short.MAX_VALUE)
         .addGroup(mons_partLayout.createSequentialGroup()
            .addContainerGap()
            .addComponent(mons_label)
            .addContainerGap(861, Short.MAX_VALUE))
      );
      mons_partLayout.setVerticalGroup(
         mons_partLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(mons_partLayout.createSequentialGroup()
            .addComponent(mons_label)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(mons_scroll_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 292, Short.MAX_VALUE))
      );

      jSplitPane1.setRightComponent(mons_part);

      jPanel1.setBorder(javax.swing.BorderFactory.createEmptyBorder(5, 5, 5, 5));
      jPanel1.setPreferredSize(new java.awt.Dimension(777, 50));

      is_multistep_reactions_check.setLabel("multistep reactions");
      is_multistep_reactions_check.addItemListener(new java.awt.event.ItemListener() {
         public void itemStateChanged(java.awt.event.ItemEvent evt) {
            is_multistep_reactions_checkItemStateChanged(evt);
         }
      });

      react_button.setText("Enumerate");
      react_button.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            react_buttonActionPerformed(evt);
         }
      });

      jTextField1.setText("1000");
      jTextField1.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            jTextField1ActionPerformed(evt);
         }
      });

      jLabel1.setText("Maximum products:");

      jLabel2.setText("Maximum number of steps:");

      jTextField2.setText("3");

      is_one_tube_check.setLabel("one tube");
      is_one_tube_check.addItemListener(new java.awt.event.ItemListener() {
         public void itemStateChanged(java.awt.event.ItemEvent evt) {
            is_one_tube_checkItemStateChanged(evt);
         }
      });

      is_self_react_check.setLabel("self reactions");
      is_self_react_check.addItemListener(new java.awt.event.ItemListener() {
         public void itemStateChanged(java.awt.event.ItemEvent evt) {
            is_self_react_checkItemStateChanged(evt);
         }
      });

      javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
      jPanel1.setLayout(jPanel1Layout);
      jPanel1Layout.setHorizontalGroup(
         jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(jPanel1Layout.createSequentialGroup()
            .addContainerGap()
            .addComponent(jLabel2)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
            .addComponent(jTextField2, javax.swing.GroupLayout.PREFERRED_SIZE, 69, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
            .addComponent(jLabel1)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(jTextField1, javax.swing.GroupLayout.PREFERRED_SIZE, 69, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 59, Short.MAX_VALUE)
            .addComponent(is_multistep_reactions_check, javax.swing.GroupLayout.PREFERRED_SIZE, 123, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(is_one_tube_check, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(is_self_react_check, javax.swing.GroupLayout.PREFERRED_SIZE, 93, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addGap(21, 21, 21)
            .addComponent(react_button, javax.swing.GroupLayout.PREFERRED_SIZE, 132, javax.swing.GroupLayout.PREFERRED_SIZE))
      );
      jPanel1Layout.setVerticalGroup(
         jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(jPanel1Layout.createSequentialGroup()
            .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
               .addComponent(react_button, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE)
               .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                  .addComponent(jTextField2, javax.swing.GroupLayout.PREFERRED_SIZE, 32, javax.swing.GroupLayout.PREFERRED_SIZE)
                  .addComponent(jLabel2, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
                  .addComponent(jLabel1, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
                  .addComponent(jTextField1, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE))
               .addComponent(is_self_react_check, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE)
               .addComponent(is_one_tube_check, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE)
               .addComponent(is_multistep_reactions_check, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE))
            .addContainerGap())
      );

      javax.swing.GroupLayout in_tabLayout = new javax.swing.GroupLayout(in_tab);
      in_tab.setLayout(in_tabLayout);
      in_tabLayout.setHorizontalGroup(
         in_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(jSplitPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 926, Short.MAX_VALUE)
         .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, 926, Short.MAX_VALUE)
      );
      in_tabLayout.setVerticalGroup(
         in_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, in_tabLayout.createSequentialGroup()
            .addComponent(jSplitPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 618, Short.MAX_VALUE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, 42, javax.swing.GroupLayout.PREFERRED_SIZE))
      );

      jTabbedPane1.addTab("Reaction", in_tab);

      out_tab.setBorder(javax.swing.BorderFactory.createEmptyBorder(5, 5, 5, 5));

      javax.swing.GroupLayout out_tabLayout = new javax.swing.GroupLayout(out_tab);
      out_tab.setLayout(out_tabLayout);
      out_tabLayout.setHorizontalGroup(
         out_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 916, Short.MAX_VALUE)
      );
      out_tabLayout.setVerticalGroup(
         out_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 656, Short.MAX_VALUE)
      );

      jTabbedPane1.addTab("Products", out_tab);

      javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
      getContentPane().setLayout(layout);
      layout.setHorizontalGroup(
         layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(jTabbedPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 931, Short.MAX_VALUE)
      );
      layout.setVerticalGroup(
         layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(jTabbedPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 694, Short.MAX_VALUE)
      );

      pack();
   }// </editor-fold>//GEN-END:initComponents

    private void reaction_buttonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_reaction_buttonActionPerformed
       JFileChooser file_chooser = new JFileChooser();
       MolFileFilter rxn_ff = new MolFileFilter();
       rxn_ff.addExtension("rxn");
       file_chooser.setFileFilter(rxn_ff);
       file_chooser.setCurrentDirectory(new File(cur_dir.dir_path));
       int ret_val = file_chooser.showOpenDialog(rct_part);
       File choosed_file = file_chooser.getSelectedFile();

       //legio.clear();

       if ((choosed_file == null) || (ret_val != JFileChooser.APPROVE_OPTION)) {
          return;
       }
       cur_dir.dir_path = choosed_file.getParent();

       String file_path = choosed_file.getAbsolutePath();
       reaction_path_label.setText(file_path);

       try {
          legio.setReactionFromFile(file_path);
       } catch (Exception ex) {
          JOptionPane msg_box = new JOptionPane();
          msg_box.showMessageDialog(this, ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
       }

       rct_view.setImageSize(rct_panel.getSize().width, rct_panel.getSize().height);
       rct_view.setMol(file_path);

       GroupLayout gl = new GroupLayout(rct_panel);
       rct_panel.setLayout(gl);

       gl.setAutoCreateGaps(true);
       gl.setHorizontalGroup(gl.createSequentialGroup().
               addComponent(rct_view, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
       gl.setVerticalGroup(gl.createSequentialGroup().
               addComponent(rct_view, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));

       int old_reactants_count = reactants_count;
       reactants_count = legio.getReactantsCount();
       int min_count;
       if (old_reactants_count <= reactants_count) {
          min_count = old_reactants_count;
          for (int i = old_reactants_count; i < reactants_count; i++) {
             mon_panels.add(new MonomerPanel(indigo, indigo_renderer, this, mons_scroll_panel, legio, i, cur_dir));
          }
       } else {
          min_count = reactants_count;
          for (int i = old_reactants_count - 1; i >= reactants_count; i--) {
             legio.clearReactantMonomers(i);
             mon_panels.remove(i);
          }
       }
       /*
       for (int i = 0; i < min_count; i++)
          if (mon_panels.get(i).mon_paths != null)
             for (int j = 0; j < mon_panels.get(i).mon_paths.size(); j++)
                if (mon_panels.get(i).mon_paths.get(j) != null)
                   legio.addMonomerFromFile(i, mon_panels.get(i).mon_paths.get(j));
       */

       mons_container.removeAll();

       /* Setting layout */
       GroupLayout gl_mc = new GroupLayout(mons_container);
       mons_container.setLayout(gl_mc);

       gl_mc.setAutoCreateGaps(true);

       Group h_group = gl_mc.createSequentialGroup();
       for (int i = 0; i < reactants_count; i++)
       {
          h_group.addComponent(mon_panels.get(i).main_panel, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE);
       }

       gl_mc.setHorizontalGroup(h_group);

       Group v_group = gl_mc.createParallelGroup(GroupLayout.Alignment.LEADING);
       for (int i = 0; i < reactants_count; i++) {
          v_group.addComponent(mon_panels.get(i).main_panel, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE);
       }
       gl_mc.setVerticalGroup(v_group);

       products_table.clear();
       mons_container.updateUI();
       save_products_button.setEnabled(false);
       save_reactions_button.setEnabled(false);
       react_button.setEnabled(true);
       is_multistep_reactions_check.setEnabled(true);
       is_one_tube_check.setEnabled(true);
       is_self_react_check.setEnabled(true);
       jLabel1.setEnabled(true);
       jTextField1.setEnabled(true);
    }//GEN-LAST:event_reaction_buttonActionPerformed

    private void react_buttonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_react_buttonActionPerformed
       try {
          int max_depth = Integer.parseInt(jTextField2.getText());
          indigo.setOption("rpe-max-depth", max_depth);
          int max_pr_count = Integer.parseInt(jTextField1.getText());
          indigo.setOption("rpe-max-products-count", max_pr_count);
          indigo.setOption("rpe-multistep-reactions", is_multistep_reactions_check.getState());
          if (is_one_tube_check.getState())
             indigo.setOption("rpe-mode", "one-tube");
          else
             indigo.setOption("rpe-mode", "grid");
          indigo.setOption("rpe-self-reaction", is_self_react_check.getState());

          legio.react();

          ArrayList<IndigoObject> mol_objects = new ArrayList<IndigoObject>();

          int mol_cnt = 0;

          for (int i = 0; i < legio.getProductsCount(); i++)
          {
             IndigoObject mol = legio.getOutReaction(i);

             mol_objects.add(mol);

             mol_cnt++;
          }

          products_table.setMols(mol_objects);
          save_products_button.setEnabled(true);
          save_reactions_button.setEnabled(true);
          jTabbedPane1.setSelectedIndex(1);
       } catch (Exception ex) {
          JOptionPane msg_box = new JOptionPane();
          msg_box.showMessageDialog((JFrame) (out_tab.getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
       }
    }//GEN-LAST:event_react_buttonActionPerformed

    private void is_multistep_reactions_checkItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_is_multistep_reactions_checkItemStateChanged
       jLabel2.setEnabled(is_multistep_reactions_check.getState());
       jTextField2.setEnabled(is_multistep_reactions_check.getState());
    }//GEN-LAST:event_is_multistep_reactions_checkItemStateChanged

    private void is_one_tube_checkItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_is_one_tube_checkItemStateChanged
       // TODO add your handling code here:
    }//GEN-LAST:event_is_one_tube_checkItemStateChanged

    private void is_self_react_checkItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_is_self_react_checkItemStateChanged
       // TODO add your handling code here:
    }//GEN-LAST:event_is_self_react_checkItemStateChanged

    private void jTextField1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jTextField1ActionPerformed
       // TODO add your handling code here:
    }//GEN-LAST:event_jTextField1ActionPerformed

   class SaveProductsEventListener implements ActionListener
   {
      public void actionPerformed(ActionEvent event) {
         try {
            JFileChooser file_chooser = new JFileChooser();
            MolFileFilter sdf_ff = new MolFileFilter();
            sdf_ff.addExtension("sdf");
            sdf_ff.addExtension("sd");
            file_chooser.setFileFilter(sdf_ff);
            file_chooser.setApproveButtonText("Save");
            file_chooser.setCurrentDirectory(new File(cur_dir.dir_path));
            int ret_val = file_chooser.showSaveDialog(out_tab);
            File out_file = file_chooser.getSelectedFile();

            if ((out_file == null) || (ret_val != JFileChooser.APPROVE_OPTION)) {
               return;
            }

            cur_dir.dir_path = out_file.getParent();

            String out_file_path = out_file.getPath();

            if (!out_file_path.endsWith(".sdf") && !out_file_path.endsWith(".sd")) {
               out_file_path += ".sdf";
            }

            FileWriter out_fstream = new FileWriter(out_file_path);

            for (int i = 0; i < legio.getProductsCount(); i++)
            {
               String product_str = legio.getOutProductString(i);

               out_fstream.write(product_str);

               out_fstream.write("$$$$\n");
            }

            out_fstream.close();
         } catch (Exception ex) {
            JOptionPane msg_box = new JOptionPane();
            msg_box.showMessageDialog((JFrame) (out_tab.getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
         }
      }
   }

   class SaveReactionsEventListener implements ActionListener
   {
      public void actionPerformed(ActionEvent event) {
         try {
            JFileChooser file_chooser = new JFileChooser();
            MolFileFilter sdf_ff = new MolFileFilter();
            sdf_ff.addExtension("rdf");
            file_chooser.setFileFilter(sdf_ff);
            file_chooser.setApproveButtonText("Save");
            file_chooser.setCurrentDirectory(new File(cur_dir.dir_path));
            int ret_val = file_chooser.showSaveDialog(out_tab);
            File out_file = file_chooser.getSelectedFile();

            if ((out_file == null) || (ret_val != JFileChooser.APPROVE_OPTION)) {
               return;
            }
            cur_dir.dir_path = out_file.getParent();

            String out_file_path = out_file.getPath();

            if (!out_file_path.endsWith(".rdf")) {
               out_file_path += ".rdf";
            }

            FileWriter out_fstream = new FileWriter(out_file_path);

            out_fstream.write("$RDFILE 1\n$DATM 1\n");
            for (int i = 0; i < legio.getProductsCount(); i++)
            {
               out_fstream.write("$RFMT\n");
               String reaction_str = legio.getOutReactionString(i);

               out_fstream.write(reaction_str);
            }

            out_fstream.close();
         } catch (Exception ex) {
            JOptionPane msg_box = new JOptionPane();
            msg_box.showMessageDialog((JFrame) (out_tab.getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
         }
      }
   }
   // Variables declaration - do not modify//GEN-BEGIN:variables
   private javax.swing.JPanel in_tab;
   private java.awt.Checkbox is_multistep_reactions_check;
   private java.awt.Checkbox is_one_tube_check;
   private java.awt.Checkbox is_self_react_check;
   private javax.swing.JLabel jLabel1;
   private javax.swing.JLabel jLabel2;
   private javax.swing.JPanel jPanel1;
   private javax.swing.JSplitPane jSplitPane1;
   private javax.swing.JTabbedPane jTabbedPane1;
   private javax.swing.JTextField jTextField1;
   private javax.swing.JTextField jTextField2;
   private javax.swing.JPanel mons_container;
   private javax.swing.JLabel mons_label;
   private javax.swing.JPanel mons_part;
   private javax.swing.JScrollPane mons_scroll_panel;
   private javax.swing.JPanel out_tab;
   private javax.swing.JPanel rct_panel;
   private javax.swing.JPanel rct_part;
   private javax.swing.JButton react_button;
   private javax.swing.JButton reaction_button;
   private javax.swing.JLabel reaction_label;
   private javax.swing.JTextField reaction_path_label;
   // End of variables declaration//GEN-END:variables
}
