/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.gui.LoadFinishEvent;
import com.ggasoftware.indigo.gui.ProgressEvent;
import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.gui.*;
import java.awt.Window;
import java.io.File;
import java.util.ArrayList;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

/**
 *
 * @author achurinov
 */
public class SdfLoader{

   private Indigo indigo;
   private File file;
   public ArrayList<MolData> mol_datas;
   private IndigoObject iterator_object;
   private Thread thread;
   private String _ext;
   public int table_idx;
   public LoadFinishEvent finish_event;
   public IndigoEventSource<ProgressEvent> progress_event = 
           new IndigoEventSource<ProgressEvent>(this);
   private SdfLoadRunnable runnable;
   public boolean arom_flag;
   public boolean cistrans_ignore_flag;
   public boolean stereocenters_ignore_flag;

   public boolean test_flag;

   public SdfLoader(Indigo cur_indigo, int cur_table_idx, String ext) {
      table_idx = cur_table_idx;

      indigo = cur_indigo;
      runnable = new SdfLoadRunnable();
      finish_event = new LoadFinishEvent(table_idx);
      mol_datas = new ArrayList<MolData>();
      thread = new Thread(null, runnable, "sdf_loader #" + table_idx, 10000000);
      _ext = ext;

      test_flag = false;
   }

   public void setExtension(String ext) {
      _ext = new String(ext);
   }
   public void setFile(File cur_file) {
      file = cur_file;
   }

   class SdfLoadRunnable implements Runnable {

      public void run() {
         mol_datas.clear();
         try {
            int file_pos = 0;

            if (_ext.equals("sdf")) {
               iterator_object = indigo.iterateSDFile(file.getPath());
            } else if (_ext.equals("smi")) {
               iterator_object = indigo.iterateSmilesFile(file.getPath());
            } else if (_ext.equals("cml")) {
               iterator_object = indigo.iterateCMLFile(file.getPath());
            } else {
               throw new Exception("Unsupported file extension");
            }

            int invalid_count = 0;
            for (IndigoObject iterr : iterator_object)
            {
               if (thread.isInterrupted())
                  return;

               file_pos = iterator_object.tell();
               try {
                  IndigoObject mol = iterr.clone();
                  if (arom_flag)
                     mol.aromatize();
                  if (cistrans_ignore_flag)
                     mol.clearCisTrans();
                  if (stereocenters_ignore_flag)
                     mol.clearStereocenters();

                  String csmiles;
                  try
                  {
                     csmiles = mol.canonicalSmiles();
                  }
                  catch (Exception ex)
                  {
                     csmiles = "unknown #" + invalid_count++ + " in table #" + table_idx;
                  }

                  mol_datas.add(new MolData(iterator_object, mol_datas.size(), csmiles));

                  if ((mol_datas.size() % 10000) == 0)
                     System.gc();

               } catch (Exception ex) {
                  mol_datas.add(new MolData(null, mol_datas.size(),
                                "unknown #" + invalid_count++ + " in table #" + table_idx));
               }

               progress_event.fireEvent(new ProgressEvent(table_idx, file_pos));
            }
         } catch (Exception ex) {
            JOptionPane msg_box = new JOptionPane();
            String msg = String.format("ChemDiff\nVersion %s\nCopyright (C) 2010-2011 GGA Software Services LLC",
                    indigo.version());
            Window[] windows = JFrame.getOwnerlessWindows();
            msg_box.showMessageDialog(windows[0], ex.getMessage(),
                    "Error loading file", JOptionPane.ERROR_MESSAGE);
         }

         finish_event.alertListeners();
      }
   }

   public void start(boolean arom_flag, boolean cistrans_ignore_flag,
                     boolean stereocenters_ignore_flag)
   {
      this.arom_flag = arom_flag;
      this.cistrans_ignore_flag = cistrans_ignore_flag;
      this.stereocenters_ignore_flag = stereocenters_ignore_flag;

      thread.start();
   }

   public boolean isActive() {
      if (thread == null)
         return false;
      return thread.isAlive();
   }

   public boolean isInterrupted() {
      if (thread == null)
         return false;
      return thread.isInterrupted();
   }

   public void interrupt() {
      if (thread == null)
         return;
      test_flag = true;
      thread.interrupt();
   }
}
