package com.epam.indigo.legio;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.IndigoRenderer;
import com.epam.indigo.controls.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.ArrayList;
import javax.swing.GroupLayout.Group;
import javax.swing.*;

public class MainFrame extends javax.swing.JFrame
{
   public static final int LARGE_HEIGHT = 250;
   private static final int COMPACT_HEIGHT = 70;
   private static final int MEDIUM_HEIGHT = 160;
   
   ArrayList<MoleculesInputTable> mon_panels;
   int reactants_count;
   LegioData legio;
   Indigo indigo;
   IndigoRenderer indigo_renderer;
   int active_row_height;

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
      indigo = Global.indigo;
      indigo_renderer = Global.indigo_renderer;

      mon_panels = new ArrayList<MoleculesInputTable>();
      legio = new LegioData(indigo);
            
      indigo.setOption("filename-encoding", "UTF-8");
      indigo.setOption("render-margins", "5,2");

      initComponents();

      //rct_view.init(indigo, indigo_renderer);

      //products_panel.init(indigo, indigo_renderer, legio);

      max_products_label.setEnabled(false);
      max_products_text_field.setEnabled(false);
      max_steps_label.setEnabled(false);
      max_steps_text_field.setEnabled(false);
      react_button.setEnabled(false);
      is_multistep_reactions_check.setEnabled(false);
      is_one_tube_check.setEnabled(false);
      is_self_react_check.setEnabled(false);
      reaction_path_label.setEditable(false);

      setRowHeight(MEDIUM_HEIGHT);
      
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
        reaction_path_label = new javax.swing.JTextField();
        rct_view = new com.epam.indigo.controls.IndigoObjectViewPanel();
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
        products_panel = new com.epam.indigo.controls.MoleculeOutputTable();
        menu_bar = new javax.swing.JMenuBar();
        file_menu = new javax.swing.JMenu();
        exit_menu_item = new javax.swing.JMenuItem();
        menu_view = new javax.swing.JMenu();
        jMenu1 = new javax.swing.JMenu();
        menu_view_compact = new javax.swing.JCheckBoxMenuItem();
        menu_view_medium = new javax.swing.JCheckBoxMenuItem();
        menu_view_large = new javax.swing.JCheckBoxMenuItem();
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

        javax.swing.GroupLayout rct_viewLayout = new javax.swing.GroupLayout(rct_view);
        rct_view.setLayout(rct_viewLayout);
        rct_viewLayout.setHorizontalGroup(
            rct_viewLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 930, Short.MAX_VALUE)
        );
        rct_viewLayout.setVerticalGroup(
            rct_viewLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
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
                .addComponent(reaction_path_label, javax.swing.GroupLayout.DEFAULT_SIZE, 773, Short.MAX_VALUE)
                .addContainerGap(18, Short.MAX_VALUE))
            .addComponent(rct_view, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
        rct_partLayout.setVerticalGroup(
            rct_partLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(rct_partLayout.createSequentialGroup()
                .addGroup(rct_partLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(reaction_label)
                    .addComponent(reaction_button, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(reaction_path_label))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(rct_view, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
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
            .addGap(0, 297, Short.MAX_VALUE)
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
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(max_steps_text_field, javax.swing.GroupLayout.PREFERRED_SIZE, 51, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(max_products_label)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(max_products_text_field, javax.swing.GroupLayout.PREFERRED_SIZE, 69, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(21, 21, 21)
                .addComponent(is_multistep_reactions_check, javax.swing.GroupLayout.PREFERRED_SIZE, 123, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(is_one_tube_check, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(is_self_react_check, javax.swing.GroupLayout.PREFERRED_SIZE, 93, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 87, Short.MAX_VALUE)
                .addComponent(react_button, javax.swing.GroupLayout.PREFERRED_SIZE, 132, javax.swing.GroupLayout.PREFERRED_SIZE))
        );
        enumeration_panelLayout.setVerticalGroup(
            enumeration_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(enumeration_panelLayout.createSequentialGroup()
                .addGroup(enumeration_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(react_button, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(enumeration_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(max_steps_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(max_steps_text_field, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(max_products_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(max_products_text_field, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(is_self_react_check, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(is_one_tube_check, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(is_multistep_reactions_check, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );

        javax.swing.GroupLayout in_tabLayout = new javax.swing.GroupLayout(in_tab);
        in_tab.setLayout(in_tabLayout);
        in_tabLayout.setHorizontalGroup(
            in_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(split_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 942, Short.MAX_VALUE)
            .addGroup(in_tabLayout.createSequentialGroup()
                .addComponent(enumeration_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 924, Short.MAX_VALUE)
                .addGap(10, 10, 10))
        );
        in_tabLayout.setVerticalGroup(
            in_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, in_tabLayout.createSequentialGroup()
                .addComponent(split_panel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(enumeration_panel, javax.swing.GroupLayout.PREFERRED_SIZE, 39, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        tabbed_panel.addTab("Reaction", in_tab);

        out_tab.setBorder(javax.swing.BorderFactory.createEmptyBorder(5, 5, 5, 5));

        products_panel.setIdColumnCount(1);
        products_panel.setReactionsContentType(true);

        javax.swing.GroupLayout out_tabLayout = new javax.swing.GroupLayout(out_tab);
        out_tab.setLayout(out_tabLayout);
        out_tabLayout.setHorizontalGroup(
            out_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(products_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 924, Short.MAX_VALUE)
        );
        out_tabLayout.setVerticalGroup(
            out_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(products_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 638, Short.MAX_VALUE)
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

        menu_view.setText("View");

        jMenu1.setText("Layout size");

        menu_view_compact.setText("Compact");
        menu_view_compact.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                menu_view_compactActionPerformed(evt);
            }
        });
        jMenu1.add(menu_view_compact);

        menu_view_medium.setSelected(true);
        menu_view_medium.setText("Medium");
        menu_view_medium.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                menu_view_mediumActionPerformed(evt);
            }
        });
        jMenu1.add(menu_view_medium);

        menu_view_large.setText("Large");
        menu_view_large.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                menu_view_largeActionPerformed(evt);
            }
        });
        jMenu1.add(menu_view_large);

        menu_view.add(jMenu1);

        menu_bar.add(menu_view);

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
            .addComponent(tabbed_panel)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void reaction_buttonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_reaction_buttonActionPerformed
        try {
            FileOpener fopener = new FileOpener();
            fopener.addExtension("rxn", "rdf", "cml", "sma", "smarts");

            if (fopener.openFile("Open") == null) {
                return;
            }

            File choosed_file = fopener.getFile();

            GlobalParams.getInstance().dir_path = choosed_file.getParent();

            String file_path = choosed_file.getAbsolutePath();
            reaction_path_label.setText(file_path);

            try {
                legio.setReactionFromFile(file_path);
            } catch (Exception ex) {
                JOptionPane.showMessageDialog(this, ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
                return;
            }

            rct_view.setIndigoObject(legio.getReaction(), indigo_renderer);

            int old_reactants_count = reactants_count;
            reactants_count = legio.getReactantsCount();
            int min_count;
            if (old_reactants_count <= reactants_count) {
                min_count = old_reactants_count;
                for (int i = old_reactants_count; i < reactants_count; i++) {
                    MoleculesInputTable input_table = new MoleculesInputTable();
                    input_table.setUseProxyObjects(false);
                    input_table.setRowHeight(active_row_height);
                    mon_panels.add(input_table);
                }
            } else {
                min_count = reactants_count;
                for (int i = old_reactants_count - 1; i >= reactants_count; i--) {
                    legio.clearReactantMonomers(i);
                    mon_panels.remove(i);
                }
            }

            mons_part.removeAll();

            /*
             * Setting layout
             */
            GroupLayout gl_mc = new GroupLayout(mons_part);
            mons_part.setLayout(gl_mc);

            gl_mc.setAutoCreateGaps(true);

            Group h_group = gl_mc.createSequentialGroup();
            Group v_group = gl_mc.createParallelGroup(GroupLayout.Alignment.LEADING);
            for (mon_panel_idx = 0; mon_panel_idx < reactants_count; mon_panel_idx++) {
                h_group.addComponent(mon_panels.get(mon_panel_idx), GroupLayout.DEFAULT_SIZE,
                        GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE);
                v_group.addComponent(mon_panels.get(mon_panel_idx), GroupLayout.DEFAULT_SIZE,
                        GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE);
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
        } catch (Exception ex) {
            StringWriter sw = new StringWriter();
            ex.printStackTrace(new PrintWriter(sw));
            String error_as_string = sw.toString();

            MessageBox.show(null, error_as_string, "Error", MessageBox.ICON_ERROR);
        }
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

          // Add reactans
          for (int i = 0; i < reactants_count; i++)
          {
              ArrayList<MoleculeItem> mols = mon_panels.get(i).getMolecules();
              legio.setMonomers(i, mols);
          }
          
          legio.react();

          ArrayList<MoleculeItem> mol_objects = new ArrayList<MoleculeItem>();

          int mol_cnt = 0;

          for (int i = 0; i < legio.getProductsCount(); i++)
          {
             IndigoObject mol = legio.getOutReaction(i);
              IndigoObjectWrapper wrapper = new PureIndigoObject(mol);

             mol_cnt++;
             mol_objects.add(new MoleculeItem(wrapper, String.format("#%d", mol_cnt)));
          }

          products_panel.setMolecules(mol_objects);
          
          tabbed_panel.setSelectedIndex(1);
       } catch (Exception ex) {
          JOptionPane.showMessageDialog((JFrame) (out_tab.getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
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
       CommonUtils.showAboutDialog(this, "Legio", "http://epam.com/opensource/indigo/legio");
    }//GEN-LAST:event_about_menu_itemActionPerformed

    private void menu_view_compactActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_menu_view_compactActionPerformed
        onLayoutSizeChanged(evt);
    }//GEN-LAST:event_menu_view_compactActionPerformed

    private void menu_view_mediumActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_menu_view_mediumActionPerformed
        onLayoutSizeChanged(evt);
    }//GEN-LAST:event_menu_view_mediumActionPerformed

    private void menu_view_largeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_menu_view_largeActionPerformed
        onLayoutSizeChanged(evt);
    }//GEN-LAST:event_menu_view_largeActionPerformed

    private void onLayoutSizeChanged(java.awt.event.ActionEvent evt) {
        menu_view_compact.setState(false);
        menu_view_medium.setState(false);
        menu_view_large.setState(false);
        if (evt.getSource() == menu_view_compact) {
            setRowHeight(COMPACT_HEIGHT);
            menu_view_compact.setState(true);
        } else if (evt.getSource() == menu_view_medium) {
            setRowHeight(MEDIUM_HEIGHT);
            menu_view_medium.setState(true);
        } else {
            setRowHeight(LARGE_HEIGHT);
            menu_view_large.setState(true);
        }
    }
    
    private void setRowHeight(int height) {
        products_panel.setRowHeight(height);
        for (MoleculesInputTable panel: mon_panels)
            panel.setRowHeight(height);
        active_row_height = height;
    }

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
            JOptionPane.showMessageDialog((JFrame) (out_tab.getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
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
            JOptionPane.showMessageDialog((JFrame) (out_tab.getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
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
    private javax.swing.JMenu jMenu1;
    private javax.swing.JLabel max_products_label;
    private javax.swing.JTextField max_products_text_field;
    private javax.swing.JLabel max_steps_label;
    private javax.swing.JTextField max_steps_text_field;
    private javax.swing.JMenuBar menu_bar;
    private javax.swing.JMenu menu_view;
    private javax.swing.JCheckBoxMenuItem menu_view_compact;
    private javax.swing.JCheckBoxMenuItem menu_view_large;
    private javax.swing.JCheckBoxMenuItem menu_view_medium;
    private javax.swing.JPanel mons_part;
    private javax.swing.JPanel out_tab;
    private com.epam.indigo.controls.MoleculeOutputTable products_panel;
    private javax.swing.JPanel rct_part;
    private com.epam.indigo.controls.IndigoObjectViewPanel rct_view;
    private javax.swing.JButton react_button;
    private javax.swing.JButton reaction_button;
    private javax.swing.JLabel reaction_label;
    private javax.swing.JTextField reaction_path_label;
    private javax.swing.JSplitPane split_panel;
    private javax.swing.JTabbedPane tabbed_panel;
    // End of variables declaration//GEN-END:variables
}
