package com.scitouch.indigo.legio;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.*;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import java.util.ArrayList;
import com.scitouch.indigo.*;

public class MonomerPanel
{
    JPanel main_panel;
    JButton add_button;
    JButton clear_button;
    LegioData legio;
    int reactant_idx;
    MolViewTable mol_table;
    Component parent;
    Frame frame;
    CurDir cur_dir;
    ArrayList<String> mon_paths;
    JTextField mon_path_label;
    Indigo indigo;
    IndigoRenderer indigo_renderer;

    public MonomerPanel( Indigo cur_indigo, IndigoRenderer cur_indigo_renderer,
             Frame cur_frame, Component parent_panel, LegioData new_legio,
             int new_reactant_idx, CurDir new_cur_dir )
    {
        indigo = cur_indigo;
        indigo_renderer = cur_indigo_renderer;
        main_panel = new JPanel();
        add_button = new JButton("Add");
        clear_button = new JButton("Clear");
        mol_table = new MolViewTable(indigo, indigo_renderer, 350, 200, false);
        mon_path_label = new JTextField();
        mon_paths = new ArrayList<String>();
        cur_dir = new_cur_dir;

        mon_path_label.setEditable(false);

        frame = cur_frame;
        legio = new_legio;
        reactant_idx = new_reactant_idx;
        parent = parent_panel;

        add_button.addActionListener(new AddMonomerEventListener());
        clear_button.addActionListener(new ClearMonomerEventListener());

        main_panel.setPreferredSize(new Dimension(350, parent_panel.getSize().height - 50));

        GroupLayout gl_mp = new GroupLayout(main_panel);
        main_panel.setLayout(gl_mp);

        gl_mp.setAutoCreateGaps(true);
        gl_mp.setHorizontalGroup(gl_mp.createParallelGroup(GroupLayout.Alignment.CENTER).
                addGroup(gl_mp.createSequentialGroup().
                   addComponent(add_button, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, 26).
                   addComponent(clear_button, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, 26)).
                 addComponent(mon_path_label, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE).
                 addComponent(mol_table, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
        gl_mp.setVerticalGroup(gl_mp.createSequentialGroup().
                addGroup(gl_mp.createParallelGroup(GroupLayout.Alignment.CENTER).
                   addComponent(add_button, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, 26).
                   addComponent(clear_button, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, 26)).
                addComponent(mon_path_label, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, 20).
                addComponent(mol_table, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
        
        main_panel.setBorder(javax.swing.BorderFactory.createEtchedBorder(new java.awt.Color(200, 200, 200),
                                                                     new java.awt.Color(150, 150, 150)));
    }

    class AddMonomerEventListener  implements ActionListener
    {
      @SuppressWarnings("static-access")
        public void actionPerformed(ActionEvent load_event)
        {
            JFileChooser file_chooser = new JFileChooser();
            MolFileFilter mon_ff = new MolFileFilter();
            mon_ff.addExtension("sdf");
            mon_ff.addExtension("sd");
            mon_ff.addExtension("mol");
            file_chooser.setFileFilter(mon_ff);
            file_chooser.setCurrentDirectory(new File(cur_dir.dir_path));
            int ret_val = file_chooser.showOpenDialog(main_panel);
            File choosed_file = file_chooser.getSelectedFile();

            if ((choosed_file == null) || (ret_val != JFileChooser.APPROVE_OPTION))
               return;

            cur_dir.dir_path = choosed_file.getParent();

            mon_paths.add(choosed_file.getAbsolutePath());

            if (mon_paths.size() == 1)
            {
               String path = "" + mon_paths.get(0);
               /*
               if (mon_paths.get(0).length() > 70)
               {
                  path += mon_paths.get(0).substring(0, 30);
                  path += "  ...  ";
                  path += mon_paths.get(0).substring(mon_paths.get(0).length() - 30,
                                                     mon_paths.get(0).length());
               }
               else
                  path += mon_paths.get(0);*/

               mon_path_label.setText(path);
            }
            else if (mon_paths.size() > 1)
               mon_path_label.setText("several files");

            try
            {
               legio.addMonomerFromFile(reactant_idx, mon_paths.get(mon_paths.size() - 1));
            } catch (Exception ex)
            {
               JOptionPane msg_box = new JOptionPane();
               msg_box.showMessageDialog(parent, ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
            }

            IndigoObject obj = new IndigoObject(indigo, legio.monomers_table.arrayAt(0).arrayAt(0).self);
            ArrayList<IndigoObject> mols = new ArrayList<IndigoObject>();
            for (int i = 0; i < legio.getMonomersCount(reactant_idx); i++)
            {
               IndigoObject mol = legio.getMonomer(reactant_idx, i);

               mols.add(mol);

            }

            mol_table.setMols(mols);
        }
    }

    class ClearMonomerEventListener  implements ActionListener
    {
       public void actionPerformed(ActionEvent load_event)
       {
           legio.clearReactantMonomers(reactant_idx);
           mol_table.clear();
           mon_paths.clear();
           mon_path_label.setText("");
       }
    }
}
