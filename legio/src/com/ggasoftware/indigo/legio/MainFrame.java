package com.ggasoftware.indigo.legio;

import javax.swing.*;
import javax.swing.GroupLayout.Group;
import java.io.*;
import java.util.ArrayList;
import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.IndigoRenderer;
import com.ggasoftware.indigo.controls.GlobalParams;
import com.ggasoftware.indigo.controls.IndigoEventListener;
import com.ggasoftware.indigo.controls.MolFileFilter;
import com.ggasoftware.indigo.controls.MolRenderer;
import com.ggasoftware.indigo.controls.MolViewPanel;
import com.ggasoftware.indigo.controls.MultiLineCellRenderer;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.net.*;

public class MainFrame extends javax.swing.JFrame
{
   ArrayList<MonomerPanel> mon_panels;
   MolViewPanel rct_view;
   int reactants_count;
   LegioData legio;
   Indigo indigo;
   IndigoRenderer indigo_renderer;

   static int mon_panel_idx;

   public static void setNativeLookAndFeel() {
      try {
         UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
      } catch (Exception e) {
         System.out.println("Error setting native LAF: " + e);
      }
   }

   public static String getPathToJarfileDir(Class classToUse) {
      String url = classToUse.getResource("/" + classToUse.getName().replaceAll("\\.", "/") + ".class").toString();
      url = url.substring(4).replaceFirst("/[^/]+\\.jar!.*$", "/");
      try {
         File dir = new File(new URL(url).toURI());
         url = dir.getAbsolutePath();
      } catch (MalformedURLException mue) {
         System.err.println(mue.getMessage());
         url = null;
      } catch (URISyntaxException ue) {
         System.err.println(ue.getMessage());
         url = null;
      }
      return url;
    }

   /* Creates new form MainFrame */
   public MainFrame() {
      String path = getPathToJarfileDir(MainFrame.class);
      if (path == null)
         indigo = new Indigo();
      else
         indigo = new Indigo(path + File.separator + "lib");
      indigo_renderer = new IndigoRenderer(indigo);
      mon_panels = new ArrayList<MonomerPanel>();
      rct_view = new MolViewPanel(indigo, indigo_renderer);
      legio = new LegioData(indigo);
      
      indigo.setOption("filename-encoding", "UTF-8");
      indigo.setOption("render-margins", "5,2");

      initComponents();
      
      products_panel.init(indigo, indigo_renderer, legio);

      max_products_label.setEnabled(false);
      max_products_text_field.setEnabled(false);
      max_steps_label.setEnabled(false);
      max_steps_text_field.setEnabled(false);
      react_button.setEnabled(false);
      is_multistep_reactions_check.setEnabled(false);
      is_one_tube_check.setEnabled(false);
      is_self_react_check.setEnabled(false);
      reaction_path_label.setEditable(false);

      setTitle("Legio");
   }

   /** This method is called from within the constructor to
    * initialize the form.
    */
   @SuppressWarnings("unchecked")
   // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
   private void initComponents() {

      tabbed_panel = new javax.swing.JTabbedPane();
      in_tab = new javax.swing.JPanel();
      split_panel = new javax.swing.JSplitPane();
      rct_part = new javax.swing.JPanel();
      reaction_label = new javax.swing.JLabel();
      reaction_button = new javax.swing.JButton();
      rct_panel = new javax.swing.JPanel();
      reaction_path_label = new javax.swing.JTextField();
      mons_part = new javax.swing.JPanel();
      enumeration_panel = new javax.swing.JPanel();
      is_multistep_reactions_check = new java.awt.Checkbox();
      react_button = new javax.swing.JButton();
      max_products_text_field = new javax.swing.JTextField();
      max_products_label = new javax.swing.JLabel();
      max_steps_label = new javax.swing.JLabel();
      max_steps_text_field = new javax.swing.JTextField();
      is_one_tube_check = new java.awt.Checkbox();
      is_self_react_check = new java.awt.Checkbox();
      out_tab = new javax.swing.JPanel();
      products_panel = new com.ggasoftware.indigo.legio.ProductsPanel();
      menu_bar = new javax.swing.JMenuBar();
      file_menu = new javax.swing.JMenu();
      exit_menu_item = new javax.swing.JMenuItem();
      help_menu = new javax.swing.JMenu();
      about_menu_item = new javax.swing.JMenuItem();

      setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

      split_panel.setDividerLocation(300);
      split_panel.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);

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
         .addGap(0, 922, Short.MAX_VALUE)
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
            .addContainerGap(18, Short.MAX_VALUE))
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

      split_panel.setTopComponent(rct_part);

      javax.swing.GroupLayout mons_partLayout = new javax.swing.GroupLayout(mons_part);
      mons_part.setLayout(mons_partLayout);
      mons_partLayout.setHorizontalGroup(
         mons_partLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 932, Short.MAX_VALUE)
      );
      mons_partLayout.setVerticalGroup(
         mons_partLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 291, Short.MAX_VALUE)
      );

      split_panel.setRightComponent(mons_part);

      enumeration_panel.setBorder(javax.swing.BorderFactory.createEmptyBorder(5, 5, 5, 5));
      enumeration_panel.setPreferredSize(new java.awt.Dimension(777, 50));

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

      max_products_text_field.setText("1000");
      max_products_text_field.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            max_products_text_fieldActionPerformed(evt);
         }
      });

      max_products_label.setText("Maximum products:");

      max_steps_label.setText("Maximum number of steps:");

      max_steps_text_field.setText("3");

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

      javax.swing.GroupLayout enumeration_panelLayout = new javax.swing.GroupLayout(enumeration_panel);
      enumeration_panel.setLayout(enumeration_panelLayout);
      enumeration_panelLayout.setHorizontalGroup(
         enumeration_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(enumeration_panelLayout.createSequentialGroup()
            .addContainerGap()
            .addComponent(max_steps_label)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
            .addComponent(max_steps_text_field, javax.swing.GroupLayout.PREFERRED_SIZE, 69, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
            .addComponent(max_products_label)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(max_products_text_field, javax.swing.GroupLayout.PREFERRED_SIZE, 69, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 67, Short.MAX_VALUE)
            .addComponent(is_multistep_reactions_check, javax.swing.GroupLayout.PREFERRED_SIZE, 123, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(is_one_tube_check, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(is_self_react_check, javax.swing.GroupLayout.PREFERRED_SIZE, 93, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addGap(21, 21, 21)
            .addComponent(react_button, javax.swing.GroupLayout.PREFERRED_SIZE, 132, javax.swing.GroupLayout.PREFERRED_SIZE))
      );
      enumeration_panelLayout.setVerticalGroup(
         enumeration_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(enumeration_panelLayout.createSequentialGroup()
            .addGroup(enumeration_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
               .addComponent(react_button, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE)
               .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, enumeration_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                  .addComponent(max_steps_text_field, javax.swing.GroupLayout.PREFERRED_SIZE, 32, javax.swing.GroupLayout.PREFERRED_SIZE)
                  .addComponent(max_steps_label, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
                  .addComponent(max_products_label, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
                  .addComponent(max_products_text_field, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE))
               .addComponent(is_self_react_check, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE)
               .addComponent(is_one_tube_check, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE)
               .addComponent(is_multistep_reactions_check, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE))
            .addContainerGap())
      );

      javax.swing.GroupLayout in_tabLayout = new javax.swing.GroupLayout(in_tab);
      in_tab.setLayout(in_tabLayout);
      in_tabLayout.setHorizontalGroup(
         in_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(split_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 934, Short.MAX_VALUE)
         .addComponent(enumeration_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 934, Short.MAX_VALUE)
      );
      in_tabLayout.setVerticalGroup(
         in_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, in_tabLayout.createSequentialGroup()
            .addComponent(split_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 597, Short.MAX_VALUE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(enumeration_panel, javax.swing.GroupLayout.PREFERRED_SIZE, 53, javax.swing.GroupLayout.PREFERRED_SIZE))
      );

      tabbed_panel.addTab("Reaction", in_tab);

      out_tab.setBorder(javax.swing.BorderFactory.createEmptyBorder(5, 5, 5, 5));

      javax.swing.GroupLayout out_tabLayout = new javax.swing.GroupLayout(out_tab);
      out_tab.setLayout(out_tabLayout);
      out_tabLayout.setHorizontalGroup(
         out_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(products_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 924, Short.MAX_VALUE)
      );
      out_tabLayout.setVerticalGroup(
         out_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(products_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 635, Short.MAX_VALUE)
      );

      tabbed_panel.addTab("Products", out_tab);

      file_menu.setText("File");

      exit_menu_item.setText("Exit");
      exit_menu_item.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            exit_menu_itemActionPerformed(evt);
         }
      });
      file_menu.add(exit_menu_item);

      menu_bar.add(file_menu);

      help_menu.setText("Help");

      about_menu_item.setText("About");
      about_menu_item.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            about_menu_itemActionPerformed(evt);
         }
      });
      help_menu.add(about_menu_item);

      menu_bar.add(help_menu);

      setJMenuBar(menu_bar);

      javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
      getContentPane().setLayout(layout);
      layout.setHorizontalGroup(
         layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(tabbed_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 939, Short.MAX_VALUE)
      );
      layout.setVerticalGroup(
         layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(tabbed_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 673, Short.MAX_VALUE)
      );

      pack();
   }// </editor-fold>//GEN-END:initComponents

    private void reaction_buttonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_reaction_buttonActionPerformed
       JFileChooser file_chooser = new JFileChooser();
       MolFileFilter rxn_ff = new MolFileFilter();
       rxn_ff.addExtension("rxn");
       file_chooser.setFileFilter(rxn_ff);
       file_chooser.setCurrentDirectory(new File(GlobalParams.getInstance().dir_path));
       int ret_val = file_chooser.showOpenDialog(rct_part);
       File choosed_file = file_chooser.getSelectedFile();

       //legio.clear();

       if ((choosed_file == null) || (ret_val != JFileChooser.APPROVE_OPTION)) {
          return;
       }
       GlobalParams.getInstance().dir_path = choosed_file.getParent();

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
             mon_panels.add(new MonomerPanel(indigo, indigo_renderer, legio, i));
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

       mons_part.removeAll();

       /* Setting layout */
       GroupLayout gl_mc = new GroupLayout(mons_part);
       mons_part.setLayout(gl_mc);

       gl_mc.setAutoCreateGaps(true);

       Group h_group = gl_mc.createSequentialGroup();
       Group v_group = gl_mc.createParallelGroup(GroupLayout.Alignment.LEADING);
       for (mon_panel_idx = 0; mon_panel_idx < reactants_count; mon_panel_idx++)
       {
          h_group.addComponent(mon_panels.get(mon_panel_idx), GroupLayout.DEFAULT_SIZE,
                               GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE);
          v_group.addComponent(mon_panels.get(mon_panel_idx), GroupLayout.DEFAULT_SIZE,
                               GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE);

          mon_panels.get(mon_panel_idx).add_event.addListener(new IndigoEventListener<String>() {
             int idx = mon_panel_idx;
             public void handleEvent(Object source, String filename) {
                legio.addMonomerFromFile(idx, filename);
             }
          });

          mon_panels.get(mon_panel_idx).clear_event.addListener(new IndigoEventListener<Integer>() {
             int idx = mon_panel_idx;
             public void handleEvent(Object source, Integer num) {
                legio.clearReactantMonomers(idx);
             }
          });
       }

       gl_mc.setHorizontalGroup(h_group);
       gl_mc.setVerticalGroup(v_group);
       
       products_panel.clear();
       mons_part.updateUI();
       react_button.setEnabled(true);
       is_multistep_reactions_check.setEnabled(true);
       is_one_tube_check.setEnabled(true);
       is_self_react_check.setEnabled(true);
       max_products_label.setEnabled(true);
       max_products_text_field.setEnabled(true);
    }//GEN-LAST:event_reaction_buttonActionPerformed

    private void react_buttonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_react_buttonActionPerformed
       try {
          int max_depth = Integer.parseInt(max_steps_text_field.getText());
          indigo.setOption("rpe-max-depth", max_depth);
          int max_pr_count = Integer.parseInt(max_products_text_field.getText());
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

          products_panel.setMols(mol_objects);
          tabbed_panel.setSelectedIndex(1);
       } catch (Exception ex) {
          JOptionPane msg_box = new JOptionPane();
          msg_box.showMessageDialog((JFrame) (out_tab.getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
       }
    }//GEN-LAST:event_react_buttonActionPerformed

    private void is_multistep_reactions_checkItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_is_multistep_reactions_checkItemStateChanged
       max_steps_label.setEnabled(is_multistep_reactions_check.getState());
       max_steps_text_field.setEnabled(is_multistep_reactions_check.getState());
    }//GEN-LAST:event_is_multistep_reactions_checkItemStateChanged

    private void is_one_tube_checkItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_is_one_tube_checkItemStateChanged
       // TODO add your handling code here:
    }//GEN-LAST:event_is_one_tube_checkItemStateChanged

    private void is_self_react_checkItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_is_self_react_checkItemStateChanged
       // TODO add your handling code here:
    }//GEN-LAST:event_is_self_react_checkItemStateChanged

    private void max_products_text_fieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_max_products_text_fieldActionPerformed
       // TODO add your handling code here:
    }//GEN-LAST:event_max_products_text_fieldActionPerformed

    private void exit_menu_itemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_exit_menu_itemActionPerformed
       dispose();
    }//GEN-LAST:event_exit_menu_itemActionPerformed

    private void about_menu_itemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_about_menu_itemActionPerformed
       JOptionPane msg_box = new JOptionPane();
       String msg = String.format("ChemDiff\nVersion %s\nCopyright (C) 2010-2011 GGA Software Services LLC",
               (new Indigo()).version());
       msg_box.showConfirmDialog(this, msg, "About", JOptionPane.DEFAULT_OPTION,
               JOptionPane.INFORMATION_MESSAGE,
               new ImageIcon("images\\logo_small.png"));
    }//GEN-LAST:event_about_menu_itemActionPerformed

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
            file_chooser.setCurrentDirectory(new File(GlobalParams.getInstance().dir_path));
            int ret_val = file_chooser.showSaveDialog(out_tab);
            File out_file = file_chooser.getSelectedFile();

            if ((out_file == null) || (ret_val != JFileChooser.APPROVE_OPTION)) {
               return;
            }

            GlobalParams.getInstance().dir_path = out_file.getParent();

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
            file_chooser.setCurrentDirectory(new File(GlobalParams.getInstance().dir_path));
            int ret_val = file_chooser.showSaveDialog(out_tab);
            File out_file = file_chooser.getSelectedFile();

            if ((out_file == null) || (ret_val != JFileChooser.APPROVE_OPTION)) {
               return;
            }
            GlobalParams.getInstance().dir_path = out_file.getParent();

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
   private javax.swing.JMenuItem about_menu_item;
   private javax.swing.JPanel enumeration_panel;
   private javax.swing.JMenuItem exit_menu_item;
   private javax.swing.JMenu file_menu;
   private javax.swing.JMenu help_menu;
   private javax.swing.JPanel in_tab;
   private java.awt.Checkbox is_multistep_reactions_check;
   private java.awt.Checkbox is_one_tube_check;
   private java.awt.Checkbox is_self_react_check;
   private javax.swing.JLabel max_products_label;
   private javax.swing.JTextField max_products_text_field;
   private javax.swing.JLabel max_steps_label;
   private javax.swing.JTextField max_steps_text_field;
   private javax.swing.JMenuBar menu_bar;
   private javax.swing.JPanel mons_part;
   private javax.swing.JPanel out_tab;
   private com.ggasoftware.indigo.legio.ProductsPanel products_panel;
   private javax.swing.JPanel rct_panel;
   private javax.swing.JPanel rct_part;
   private javax.swing.JButton react_button;
   private javax.swing.JButton reaction_button;
   private javax.swing.JLabel reaction_label;
   private javax.swing.JTextField reaction_path_label;
   private javax.swing.JSplitPane split_panel;
   private javax.swing.JTabbedPane tabbed_panel;
   // End of variables declaration//GEN-END:variables
}
