package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.*;
import java.awt.Dimension;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.GroupLayout;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JProgressBar;
import javax.swing.JTabbedPane;
import java.net.*;
import javax.swing.UIManager;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * MainFrame.java
 *
 * Created on Apr 2, 2010, 2:45:57 PM
 */

/**
 *
 * @author achurinov
 */
public class MainFrame extends javax.swing.JFrame 
{
   MolViewTable in_table1;
   MolViewTable in_table2;
   MolViewTable out_table1;
   MolViewTable out_table2;
   MolViewTable out_table3;
   CurDir cur_dir;
   MolComparer mol_comparer;
   Indigo indigo;
   IndigoRenderer indigo_renderer;

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

   /** Creates new form MainFrame */
   public MainFrame()
   {
      try {
      //   UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
      }
      catch (Exception e) {}
      
      String path = getPathToJarfileDir(MainFrame.class);
      if (path == null)
         indigo = new Indigo();
      else
         indigo = new Indigo(path + File.separator + "lib");
      indigo_renderer = new IndigoRenderer(indigo);
      //indigo.setOption("ignore-stereochemistry-errors", true);
      indigo.setOption("render-margins", "5,2");
      initComponents();
      
      mol_comparer = new MolComparer(this);

      in_table1 = new MolViewTable(indigo, indigo_renderer, this, 0, 300, 150, false);
      in_table2 = new MolViewTable(indigo, indigo_renderer, this, 1, 300, 150, false);
      out_table1 = new MolViewTable(indigo, indigo_renderer, this, -1, 200, 150, false);
      out_table2 = new MolViewTable(indigo, indigo_renderer, this, -1, 200, 150, false);
      out_table3 = new MolViewTable(indigo, indigo_renderer, this, -1, 200, 150, false);
      cur_dir = new CurDir();

      GroupLayout in_gl1 = new GroupLayout(in_table_panel1);
      in_table_panel1.setLayout(in_gl1);

      in_gl1.setAutoCreateGaps(true);
      in_gl1.setHorizontalGroup(in_gl1.createParallelGroup(GroupLayout.Alignment.LEADING).
               addComponent(in_table1, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
      in_gl1.setVerticalGroup(in_gl1.createSequentialGroup().
               addComponent(in_table1, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));

      GroupLayout in_gl2 = new GroupLayout(in_table_panel2);
      in_table_panel2.setLayout(in_gl2);
       
      in_gl2.setAutoCreateGaps(true);
      in_gl2.setHorizontalGroup(in_gl2.createParallelGroup(GroupLayout.Alignment.LEADING).
               addComponent(in_table2, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
      in_gl2.setVerticalGroup(in_gl2.createSequentialGroup().
               addComponent(in_table2, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));

      GroupLayout out_gl1 = new GroupLayout(out_table_panel1);
      out_table_panel1.setLayout(out_gl1);

      out_gl1.setAutoCreateGaps(true);
      out_gl1.setHorizontalGroup(out_gl1.createParallelGroup(GroupLayout.Alignment.LEADING).
               addComponent(out_table1, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
      out_gl1.setVerticalGroup(out_gl1.createSequentialGroup().
               addComponent(out_table1, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));

      GroupLayout out_gl2 = new GroupLayout(out_table_panel2);
      out_table_panel2.setLayout(out_gl2);
        
      out_gl2.setAutoCreateGaps(true);
      out_gl2.setHorizontalGroup(out_gl2.createParallelGroup(GroupLayout.Alignment.LEADING).
               addComponent(out_table2, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
      out_gl2.setVerticalGroup(out_gl2.createSequentialGroup().
               addComponent(out_table2, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));

      GroupLayout out_gl3 = new GroupLayout(out_table_panel3);
      out_table_panel3.setLayout(out_gl3);
      out_gl3.setAutoCreateGaps(true);
      out_gl3.setHorizontalGroup(out_gl3.createParallelGroup(GroupLayout.Alignment.LEADING).
               addComponent(out_table3, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
      out_gl3.setVerticalGroup(out_gl3.createSequentialGroup().
               addComponent(out_table3, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));

      setTitle("ChemDiff");
   }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
   // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
   private void initComponents() {

      jTabbedPane1 = new javax.swing.JTabbedPane();
      in_tab = new javax.swing.JPanel();
      in_panel1 = new javax.swing.JPanel();
      in_table_panel1 = new javax.swing.JPanel();
      load_progress_bar1 = new javax.swing.JProgressBar();
      jPanel1 = new javax.swing.JPanel();
      load_first = new javax.swing.JButton();
      in_panel2 = new javax.swing.JPanel();
      in_table_panel2 = new javax.swing.JPanel();
      load_progress_bar2 = new javax.swing.JProgressBar();
      jPanel2 = new javax.swing.JPanel();
      load_second = new javax.swing.JButton();
      filler1 = new javax.swing.Box.Filler(new java.awt.Dimension(0, 0), new java.awt.Dimension(0, 0), new java.awt.Dimension(32767, 32767));
      action_panel = new javax.swing.JPanel();
      compare_button = new javax.swing.JButton();
      main_progress_bar = new javax.swing.JProgressBar();
      out_tab = new javax.swing.JPanel();
      out_panel1 = new javax.swing.JPanel();
      out_tabel_label1 = new javax.swing.JLabel();
      out_table_panel1 = new javax.swing.JPanel();
      save_panel1 = new javax.swing.JPanel();
      save_button1 = new javax.swing.JButton();
      out_panel2 = new javax.swing.JPanel();
      out_tabel_label2 = new javax.swing.JLabel();
      out_table_panel2 = new javax.swing.JPanel();
      save_panel2 = new javax.swing.JPanel();
      save_button2 = new javax.swing.JButton();
      out_panel3 = new javax.swing.JPanel();
      out_tabel_label3 = new javax.swing.JLabel();
      out_table_panel3 = new javax.swing.JPanel();
      save_panel3 = new javax.swing.JPanel();
      save_button3 = new javax.swing.JButton();
      jMainMenuBar = new javax.swing.JMenuBar();
      jMenuFile = new javax.swing.JMenu();
      jMenuLoadLeft = new javax.swing.JMenuItem();
      jMenuLoadRight = new javax.swing.JMenuItem();
      jSeparator1 = new javax.swing.JPopupMenu.Separator();
      jMenuExit = new javax.swing.JMenuItem();
      jMenuOptions = new javax.swing.JMenu();
      aromatizer_check = new javax.swing.JCheckBoxMenuItem();
      stereocenters_check = new javax.swing.JCheckBoxMenuItem();
      cistrans_check = new javax.swing.JCheckBoxMenuItem();
      jMenuHelp = new javax.swing.JMenu();
      jMenuHelpAbout = new javax.swing.JMenuItem();

      setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

      jTabbedPane1.setPreferredSize(new java.awt.Dimension(811, 910));

      in_tab.setBorder(javax.swing.BorderFactory.createEmptyBorder(5, 5, 5, 5));
      in_tab.setPreferredSize(new java.awt.Dimension(660, 760));

      in_panel1.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
      in_panel1.setPreferredSize(new java.awt.Dimension(300, 670));

      in_table_panel1.setPreferredSize(new java.awt.Dimension(350, 617));

      javax.swing.GroupLayout in_table_panel1Layout = new javax.swing.GroupLayout(in_table_panel1);
      in_table_panel1.setLayout(in_table_panel1Layout);
      in_table_panel1Layout.setHorizontalGroup(
         in_table_panel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 418, Short.MAX_VALUE)
      );
      in_table_panel1Layout.setVerticalGroup(
         in_table_panel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 461, Short.MAX_VALUE)
      );

      load_first.setText("Load the first file");
      load_first.setMaximumSize(new java.awt.Dimension(160, 26));
      load_first.setMinimumSize(new java.awt.Dimension(160, 26));
      load_first.setPreferredSize(new java.awt.Dimension(160, 26));
      load_first.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            load_firstActionPerformed(evt);
         }
      });
      jPanel1.add(load_first);

      javax.swing.GroupLayout in_panel1Layout = new javax.swing.GroupLayout(in_panel1);
      in_panel1.setLayout(in_panel1Layout);
      in_panel1Layout.setHorizontalGroup(
         in_panel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(in_table_panel1, javax.swing.GroupLayout.DEFAULT_SIZE, 418, Short.MAX_VALUE)
         .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, 418, Short.MAX_VALUE)
         .addComponent(load_progress_bar1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 418, Short.MAX_VALUE)
      );
      in_panel1Layout.setVerticalGroup(
         in_panel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(in_panel1Layout.createSequentialGroup()
            .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, 36, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(load_progress_bar1, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(in_table_panel1, javax.swing.GroupLayout.DEFAULT_SIZE, 461, Short.MAX_VALUE))
      );

      in_panel2.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
      in_panel2.setPreferredSize(new java.awt.Dimension(300, 670));

      in_table_panel2.setPreferredSize(new java.awt.Dimension(350, 617));

      javax.swing.GroupLayout in_table_panel2Layout = new javax.swing.GroupLayout(in_table_panel2);
      in_table_panel2.setLayout(in_table_panel2Layout);
      in_table_panel2Layout.setHorizontalGroup(
         in_table_panel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 413, Short.MAX_VALUE)
      );
      in_table_panel2Layout.setVerticalGroup(
         in_table_panel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 462, Short.MAX_VALUE)
      );

      load_second.setText("Load the second file");
      load_second.setMaximumSize(new java.awt.Dimension(160, 26));
      load_second.setMinimumSize(new java.awt.Dimension(160, 26));
      load_second.setPreferredSize(new java.awt.Dimension(160, 26));
      load_second.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            load_secondActionPerformed(evt);
         }
      });
      jPanel2.add(load_second);
      jPanel2.add(filler1);

      javax.swing.GroupLayout in_panel2Layout = new javax.swing.GroupLayout(in_panel2);
      in_panel2.setLayout(in_panel2Layout);
      in_panel2Layout.setHorizontalGroup(
         in_panel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, 413, Short.MAX_VALUE)
         .addComponent(load_progress_bar2, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 413, Short.MAX_VALUE)
         .addComponent(in_table_panel2, javax.swing.GroupLayout.DEFAULT_SIZE, 413, Short.MAX_VALUE)
      );
      in_panel2Layout.setVerticalGroup(
         in_panel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(in_panel2Layout.createSequentialGroup()
            .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(load_progress_bar2, javax.swing.GroupLayout.PREFERRED_SIZE, 19, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(in_table_panel2, javax.swing.GroupLayout.DEFAULT_SIZE, 462, Short.MAX_VALUE))
      );

      compare_button.setText("Compare");
      compare_button.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            compare_buttonActionPerformed(evt);
         }
      });

      javax.swing.GroupLayout action_panelLayout = new javax.swing.GroupLayout(action_panel);
      action_panel.setLayout(action_panelLayout);
      action_panelLayout.setHorizontalGroup(
         action_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, action_panelLayout.createSequentialGroup()
            .addComponent(main_progress_bar, javax.swing.GroupLayout.DEFAULT_SIZE, 740, Short.MAX_VALUE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(compare_button, javax.swing.GroupLayout.PREFERRED_SIZE, 99, javax.swing.GroupLayout.PREFERRED_SIZE))
      );
      action_panelLayout.setVerticalGroup(
         action_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(main_progress_bar, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE)
         .addComponent(compare_button, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE)
      );

      javax.swing.GroupLayout in_tabLayout = new javax.swing.GroupLayout(in_tab);
      in_tab.setLayout(in_tabLayout);
      in_tabLayout.setHorizontalGroup(
         in_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(in_tabLayout.createSequentialGroup()
            .addComponent(in_panel1, javax.swing.GroupLayout.DEFAULT_SIZE, 422, Short.MAX_VALUE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(in_panel2, javax.swing.GroupLayout.DEFAULT_SIZE, 417, Short.MAX_VALUE))
         .addComponent(action_panel, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
      );
      in_tabLayout.setVerticalGroup(
         in_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(in_tabLayout.createSequentialGroup()
            .addGroup(in_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
               .addComponent(in_panel2, javax.swing.GroupLayout.DEFAULT_SIZE, 533, Short.MAX_VALUE)
               .addComponent(in_panel1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 533, Short.MAX_VALUE))
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(action_panel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
      );

      jTabbedPane1.addTab("input", in_tab);

      out_tab.setBorder(javax.swing.BorderFactory.createEmptyBorder(5, 5, 5, 5));
      out_tab.setLayout(new java.awt.GridLayout(1, 0, 3, 3));

      out_panel1.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
      out_panel1.setPreferredSize(new java.awt.Dimension(350, 670));

      out_tabel_label1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
      out_tabel_label1.setText("Coincident Molecules");
      out_tabel_label1.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

      out_table_panel1.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));
      out_table_panel1.setPreferredSize(new java.awt.Dimension(350, 617));

      javax.swing.GroupLayout out_table_panel1Layout = new javax.swing.GroupLayout(out_table_panel1);
      out_table_panel1.setLayout(out_table_panel1Layout);
      out_table_panel1Layout.setHorizontalGroup(
         out_table_panel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 273, Short.MAX_VALUE)
      );
      out_table_panel1Layout.setVerticalGroup(
         out_table_panel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 495, Short.MAX_VALUE)
      );

      save_button1.setText("Save");
      save_button1.setMaximumSize(new java.awt.Dimension(120, 26));
      save_button1.setMinimumSize(new java.awt.Dimension(120, 26));
      save_button1.setPreferredSize(new java.awt.Dimension(120, 26));
      save_button1.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            save_button1ActionPerformed(evt);
         }
      });
      save_panel1.add(save_button1);

      javax.swing.GroupLayout out_panel1Layout = new javax.swing.GroupLayout(out_panel1);
      out_panel1.setLayout(out_panel1Layout);
      out_panel1Layout.setHorizontalGroup(
         out_panel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(out_tabel_label1, javax.swing.GroupLayout.DEFAULT_SIZE, 275, Short.MAX_VALUE)
         .addComponent(save_panel1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 275, Short.MAX_VALUE)
         .addComponent(out_table_panel1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 275, Short.MAX_VALUE)
      );
      out_panel1Layout.setVerticalGroup(
         out_panel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(out_panel1Layout.createSequentialGroup()
            .addComponent(out_tabel_label1, javax.swing.GroupLayout.PREFERRED_SIZE, 22, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(out_table_panel1, javax.swing.GroupLayout.DEFAULT_SIZE, 497, Short.MAX_VALUE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(save_panel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
      );

      out_tab.add(out_panel1);

      out_panel2.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
      out_panel2.setPreferredSize(new java.awt.Dimension(350, 670));

      out_tabel_label2.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
      out_tabel_label2.setText("Unique Molecules from the First File");
      out_tabel_label2.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

      out_table_panel2.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));
      out_table_panel2.setPreferredSize(new java.awt.Dimension(350, 617));

      javax.swing.GroupLayout out_table_panel2Layout = new javax.swing.GroupLayout(out_table_panel2);
      out_table_panel2.setLayout(out_table_panel2Layout);
      out_table_panel2Layout.setHorizontalGroup(
         out_table_panel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 273, Short.MAX_VALUE)
      );
      out_table_panel2Layout.setVerticalGroup(
         out_table_panel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 496, Short.MAX_VALUE)
      );

      save_button2.setText("Save");
      save_button2.setMaximumSize(new java.awt.Dimension(120, 26));
      save_button2.setMinimumSize(new java.awt.Dimension(120, 26));
      save_button2.setPreferredSize(new java.awt.Dimension(120, 26));
      save_button2.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            save_button2ActionPerformed(evt);
         }
      });
      save_panel2.add(save_button2);

      javax.swing.GroupLayout out_panel2Layout = new javax.swing.GroupLayout(out_panel2);
      out_panel2.setLayout(out_panel2Layout);
      out_panel2Layout.setHorizontalGroup(
         out_panel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(out_tabel_label2, javax.swing.GroupLayout.DEFAULT_SIZE, 275, Short.MAX_VALUE)
         .addComponent(out_table_panel2, javax.swing.GroupLayout.DEFAULT_SIZE, 275, Short.MAX_VALUE)
         .addComponent(save_panel2, javax.swing.GroupLayout.DEFAULT_SIZE, 275, Short.MAX_VALUE)
      );
      out_panel2Layout.setVerticalGroup(
         out_panel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(out_panel2Layout.createSequentialGroup()
            .addComponent(out_tabel_label2, javax.swing.GroupLayout.PREFERRED_SIZE, 22, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(out_table_panel2, javax.swing.GroupLayout.DEFAULT_SIZE, 498, Short.MAX_VALUE)
            .addGap(5, 5, 5)
            .addComponent(save_panel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
      );

      out_tab.add(out_panel2);

      out_panel3.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
      out_panel3.setPreferredSize(new java.awt.Dimension(350, 670));

      out_tabel_label3.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
      out_tabel_label3.setText("Unique Molecules from the Second File");
      out_tabel_label3.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

      out_table_panel3.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));
      out_table_panel3.setPreferredSize(new java.awt.Dimension(350, 617));

      javax.swing.GroupLayout out_table_panel3Layout = new javax.swing.GroupLayout(out_table_panel3);
      out_table_panel3.setLayout(out_table_panel3Layout);
      out_table_panel3Layout.setHorizontalGroup(
         out_table_panel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 273, Short.MAX_VALUE)
      );
      out_table_panel3Layout.setVerticalGroup(
         out_table_panel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 495, Short.MAX_VALUE)
      );

      save_button3.setText("Save");
      save_button3.setMaximumSize(new java.awt.Dimension(120, 26));
      save_button3.setMinimumSize(new java.awt.Dimension(120, 26));
      save_button3.setPreferredSize(new java.awt.Dimension(120, 26));
      save_button3.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            save_button3ActionPerformed(evt);
         }
      });
      save_panel3.add(save_button3);

      javax.swing.GroupLayout out_panel3Layout = new javax.swing.GroupLayout(out_panel3);
      out_panel3.setLayout(out_panel3Layout);
      out_panel3Layout.setHorizontalGroup(
         out_panel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(out_tabel_label3, javax.swing.GroupLayout.DEFAULT_SIZE, 275, Short.MAX_VALUE)
         .addComponent(out_table_panel3, javax.swing.GroupLayout.DEFAULT_SIZE, 275, Short.MAX_VALUE)
         .addComponent(save_panel3, javax.swing.GroupLayout.DEFAULT_SIZE, 275, Short.MAX_VALUE)
      );
      out_panel3Layout.setVerticalGroup(
         out_panel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(out_panel3Layout.createSequentialGroup()
            .addComponent(out_tabel_label3, javax.swing.GroupLayout.PREFERRED_SIZE, 22, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(out_table_panel3, javax.swing.GroupLayout.DEFAULT_SIZE, 497, Short.MAX_VALUE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(save_panel3, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
      );

      out_tab.add(out_panel3);

      jTabbedPane1.addTab("output", out_tab);

      jMenuFile.setText("File");
      jMenuFile.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            load_secondActionPerformed(evt);
         }
      });

      jMenuLoadLeft.setText("Load first");
      jMenuLoadLeft.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            load_firstActionPerformed(evt);
         }
      });
      jMenuFile.add(jMenuLoadLeft);

      jMenuLoadRight.setText("Load second");
      jMenuLoadRight.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            load_secondActionPerformed(evt);
         }
      });
      jMenuFile.add(jMenuLoadRight);
      jMenuFile.add(jSeparator1);

      jMenuExit.setText("Exit");
      jMenuExit.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            jMenuExitActionPerformed(evt);
         }
      });
      jMenuFile.add(jMenuExit);

      jMainMenuBar.add(jMenuFile);

      jMenuOptions.setText("Options");

      aromatizer_check.setSelected(true);
      aromatizer_check.setText("Aromatize molecules");
      jMenuOptions.add(aromatizer_check);

      stereocenters_check.setText("Ignore stereocenters");
      jMenuOptions.add(stereocenters_check);

      cistrans_check.setText("Ignore cis-trans bonds");
      jMenuOptions.add(cistrans_check);

      jMainMenuBar.add(jMenuOptions);

      jMenuHelp.setText("Help");

      jMenuHelpAbout.setText("About");
      jMenuHelpAbout.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            jMenuHelpAboutActionPerformed(evt);
         }
      });
      jMenuHelp.add(jMenuHelpAbout);

      jMainMenuBar.add(jMenuHelp);

      setJMenuBar(jMainMenuBar);

      javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
      getContentPane().setLayout(layout);
      layout.setHorizontalGroup(
         layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(jTabbedPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 860, Short.MAX_VALUE)
      );
      layout.setVerticalGroup(
         layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addComponent(jTabbedPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 609, Short.MAX_VALUE)
      );

      pack();
   }// </editor-fold>//GEN-END:initComponents

    private void load_firstActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_load_firstActionPerformed
       String path = in_table1.openSdf(cur_dir);
       if (path != null)
       {
          load_progress_bar1.setString(path);
          load_progress_bar1.setStringPainted(true);
       }
    }//GEN-LAST:event_load_firstActionPerformed

    private void load_secondActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_load_secondActionPerformed
       String path = in_table2.openSdf(cur_dir);
       if (path != null)
       {
          load_progress_bar2.setString(path);
          load_progress_bar2.setStringPainted(true);
       }
    }//GEN-LAST:event_load_secondActionPerformed

    private void save_button1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_save_button1ActionPerformed
       out_table1.saveSdf(cur_dir);
    }//GEN-LAST:event_save_button1ActionPerformed

    private void compare_buttonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_compare_buttonActionPerformed
       try {
          mol_comparer = new MolComparer(this);
          mol_comparer.setMols(in_table1.molecules, 0);
          mol_comparer.setMols(in_table2.molecules, 1);

          mol_comparer.compare();
       } catch (Exception ex) {
          JOptionPane msg_box = new JOptionPane();
          msg_box.showMessageDialog((JFrame) (out_tab.getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
       }
    }//GEN-LAST:event_compare_buttonActionPerformed

    private void save_button2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_save_button2ActionPerformed
       out_table2.saveSdf(cur_dir);
    }//GEN-LAST:event_save_button2ActionPerformed

    private void save_button3ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_save_button3ActionPerformed
       out_table3.saveSdf(cur_dir);
    }//GEN-LAST:event_save_button3ActionPerformed

    private void jMenuExitActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuExitActionPerformed
       dispose();
    }//GEN-LAST:event_jMenuExitActionPerformed

    private void jMenuHelpAboutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuHelpAboutActionPerformed
       JOptionPane msg_box = new JOptionPane();
       String msg = String.format("ChemDiff\nVersion %s\nCopyright (C) 2010-2011 GGA Software Services LLC", 
               indigo.version());
       msg_box.showMessageDialog(this, msg, "About", JOptionPane.INFORMATION_MESSAGE);
    }//GEN-LAST:event_jMenuHelpAboutActionPerformed

    /**
    * @param args the command line arguments
    */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new MainFrame().setVisible(true);
            }
        });
    }

    public JProgressBar getMainProgressBar()
    {
       return main_progress_bar;
    }
    
    public JProgressBar getLoadProgressBar( int table_idx )
    {
       if (table_idx == 0)
          return load_progress_bar1;
       else
          return load_progress_bar2;
    }
    
    public JButton getCompareButton()
    {
       return compare_button;
    }

    public JButton getLoadButton( int table_idx )
    {
       if (table_idx == 0)
          return load_first;
       else
          return load_second;
    }

    public MolViewTable getInputTable( int table_idx )
    {
       if (table_idx == 0)
          return in_table1;
       else
          return in_table2;
    }

    public MolViewTable getOutputTable( int table_idx )
    {
       if (table_idx == 0)
          return out_table1;
       else if (table_idx == 1)
          return out_table2;
       else
          return out_table3;
    }

    public JTabbedPane getTabPanel()
    {
       return jTabbedPane1;
    }

    public boolean getAromatizeCheckState()
    {
       return aromatizer_check.getState();
    }

    public boolean getStereocentersCheckState()
    {
       return stereocenters_check.getState();
    }

    public boolean getCisTransCheckState()
    {
       return cistrans_check.getState();
    }

   // Variables declaration - do not modify//GEN-BEGIN:variables
   private javax.swing.JPanel action_panel;
   private javax.swing.JCheckBoxMenuItem aromatizer_check;
   private javax.swing.JCheckBoxMenuItem cistrans_check;
   private javax.swing.JButton compare_button;
   private javax.swing.Box.Filler filler1;
   private javax.swing.JPanel in_panel1;
   private javax.swing.JPanel in_panel2;
   private javax.swing.JPanel in_tab;
   private javax.swing.JPanel in_table_panel1;
   private javax.swing.JPanel in_table_panel2;
   private javax.swing.JMenuBar jMainMenuBar;
   private javax.swing.JMenuItem jMenuExit;
   private javax.swing.JMenu jMenuFile;
   private javax.swing.JMenu jMenuHelp;
   private javax.swing.JMenuItem jMenuHelpAbout;
   private javax.swing.JMenuItem jMenuLoadLeft;
   private javax.swing.JMenuItem jMenuLoadRight;
   private javax.swing.JMenu jMenuOptions;
   private javax.swing.JPanel jPanel1;
   private javax.swing.JPanel jPanel2;
   private javax.swing.JPopupMenu.Separator jSeparator1;
   private javax.swing.JTabbedPane jTabbedPane1;
   private javax.swing.JButton load_first;
   private javax.swing.JProgressBar load_progress_bar1;
   private javax.swing.JProgressBar load_progress_bar2;
   private javax.swing.JButton load_second;
   private javax.swing.JProgressBar main_progress_bar;
   private javax.swing.JPanel out_panel1;
   private javax.swing.JPanel out_panel2;
   private javax.swing.JPanel out_panel3;
   private javax.swing.JPanel out_tab;
   private javax.swing.JLabel out_tabel_label1;
   private javax.swing.JLabel out_tabel_label2;
   private javax.swing.JLabel out_tabel_label3;
   private javax.swing.JPanel out_table_panel1;
   private javax.swing.JPanel out_table_panel2;
   private javax.swing.JPanel out_table_panel3;
   private javax.swing.JButton save_button1;
   private javax.swing.JButton save_button2;
   private javax.swing.JButton save_button3;
   private javax.swing.JPanel save_panel1;
   private javax.swing.JPanel save_panel2;
   private javax.swing.JPanel save_panel3;
   private javax.swing.JCheckBoxMenuItem stereocenters_check;
   // End of variables declaration//GEN-END:variables
}
