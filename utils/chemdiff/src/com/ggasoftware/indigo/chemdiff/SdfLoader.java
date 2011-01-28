/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.nio.CharBuffer;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JButton;
import javax.swing.JOptionPane;
import javax.swing.JProgressBar;
import javax.swing.JTable;

/**
 *
 * @author achurinov
 */
public class SdfLoader {

   private Indigo indigo;
   private File file;
   private ArrayList<IndigoObject> molecules;
   private SdfLoadRunnable runnable;
   private Thread thread;
   private String _ext;
   private MainFrame main_frame;
   private int table_idx;

   public SdfLoader(Indigo cur_indigo, MainFrame new_main_frame, int cur_table_idx, String ext) {
      main_frame = new_main_frame;
      table_idx = cur_table_idx;

      indigo = cur_indigo;
      molecules = new ArrayList<IndigoObject>();
      runnable = new SdfLoadRunnable();
      thread = new Thread(null, runnable, "sdf_loader #" + table_idx, 10000000);
      _ext = ext;
   }

   public void setFile(File cur_file) {
      file = cur_file;
   }

   class SdfLoadRunnable implements Runnable {
      // This method is called when the thread runs

      public void run() {
         main_frame.getInputTable(table_idx).setVisible(false);
         main_frame.getLoadButton(table_idx).setEnabled(false);
         main_frame.getCompareButton().setEnabled(false);

         molecules.clear();
         try {
            int file_pos = 0;
            int old_cnt = 0;
            long old_time = 0;

            IndigoObject iterator_object;

            if (_ext == "sdf") {
               iterator_object = indigo.iterateSDFile(file.getPath());
            } else if (_ext == "smi") {
               iterator_object = indigo.iterateSmilesFile(file.getPath());
            } else if (_ext == "cml") {
               //iterator_object = indigo.iterateCML(file.getPath());
               throw new Exception("CML isn't supported yet");
            } else {
               throw new Exception("Unsupported file extension");
            }

            for (IndigoObject iterr : iterator_object) {
               file_pos = iterator_object.tell();

               try {
                  molecules.add(iterr.clone());
               } catch (Exception ex) {
                  molecules.add(null);
               }

               main_frame.getLoadProgressBar(table_idx).setValue(file_pos);
            }
         } catch (Exception ex) {
            JOptionPane msg_box = new JOptionPane();
            String msg = String.format("ChemDiff\nVersion %s\nCopyright (C) 2010-2011 GGA Software Services LLC",
                    indigo.version());
            msg_box.showMessageDialog(main_frame, ex.getMessage(), 
                    "Error loading file", JOptionPane.ERROR_MESSAGE);
         }
         main_frame.getInputTable(table_idx).setMols(molecules);
         main_frame.getLoadButton(table_idx).setEnabled(true);
         if (!main_frame.getInputTable((table_idx + 1) % 2).getSdfLoader().isActive()) {
            main_frame.getCompareButton().setEnabled(true);
         }
         main_frame.getInputTable(table_idx).setVisible(true);
      }
   }

   public void start() {
      thread.start();
   }

   public void interrupt() {
      thread.interrupt();
   }

   public boolean isActive() {
      return thread.isAlive();
   }
}
