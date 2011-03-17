/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.controls.*;
import java.awt.Window;
import java.io.File;
import java.util.ArrayList;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

public class SdfLoader{

   private Indigo indigo;
   private File file;
   public ArrayList<MolData> mol_datas;
   private IndigoObject iterator_object;
   private Thread thread;
   private String _ext;

   public IndigoEventSource<Integer> finish_event =
           new IndigoEventSource<Integer>(this);
   public IndigoEventSource<Integer> progress_event =
           new IndigoEventSource<Integer>(this);
   private SdfLoadRunnable runnable;

   public boolean test_flag;

   public SdfLoader(Indigo cur_indigo, String ext)
   {
      indigo = cur_indigo;
      runnable = new SdfLoadRunnable();
      finish_event = new IndigoEventSource<Integer>(this);
      mol_datas = new ArrayList<MolData>();
      thread = new Thread(null, runnable, "sdf_loader #", 10000000);
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

            for (IndigoObject iterr : iterator_object)
            {
               synchronized (iterr)
               {
                  if (thread.isInterrupted())
                     return;

                  file_pos = iterator_object.tell();

                  mol_datas.add(new MolData(iterator_object, mol_datas.size()));

                  if ((mol_datas.size() % 10000) == 0)
                     System.gc();
               }

               progress_event.fireEvent(file_pos);
            }
         } catch (Exception ex) {
            JOptionPane msg_box = new JOptionPane();
            String msg = String.format("ChemDiff\nVersion %s\nCopyright (C) 2010-2011 GGA Software Services LLC",
                    indigo.version());
            Window[] windows = JFrame.getOwnerlessWindows();
            msg_box.showMessageDialog(windows[0], ex.getMessage(),
                    "Error loading file", JOptionPane.ERROR_MESSAGE);
         }

         finish_event.fireEvent(null);
      }
   }

   public void start()
   {
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
