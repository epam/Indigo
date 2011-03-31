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
import com.ggasoftware.indigo.controls.CommonUtils;

public class SdfLoader{

   private Indigo _indigo;
   private File _file;
   public ArrayList<RenderableMolData> mol_datas;
   private IndigoObject _iterator_object;
   private Thread _thread;
   private String _ext;

   public IndigoEventSource<Integer> finish_event =
           new IndigoEventSource<Integer>(this);
   public IndigoEventSource<Integer> progress_event =
           new IndigoEventSource<Integer>(this);
   private SdfLoadRunnable _runnable;

   public SdfLoader(Indigo cur_indigo, String ext)
   {
      _indigo = cur_indigo;
      _runnable = new SdfLoadRunnable();
      finish_event = new IndigoEventSource<Integer>(this);
      mol_datas = new ArrayList<RenderableMolData>();
      _thread = new Thread(null, _runnable, "sdf_loader #", 10000000);
      _ext = ext;
   }

   public void setExtension(String ext) {
      _ext = new String(ext);
   }

   public void setFile(File cur_file) {
      _file = cur_file;
   }

   class SdfLoadRunnable implements Runnable {
      public void run() {
         mol_datas.clear();
         try {
            int file_pos = 0;

            _iterator_object = CommonUtils.getIterator(_indigo, _file.getPath());

            for (IndigoObject iterr : _iterator_object)
            {
               synchronized (iterr)
               {
                  if (_thread.isInterrupted())
                     return;

                  file_pos = _iterator_object.tell();

                  mol_datas.add(new RenderableMolData(_iterator_object, mol_datas.size()));

                  if ((mol_datas.size() % 10000) == 0)
                     System.gc();
               }

               progress_event.fireEvent(file_pos);
            }
         } catch (Exception ex) {
            JOptionPane msg_box = new JOptionPane();
            String msg = String.format("ChemDiff\nVersion %s\nCopyright (C) 2010-2011 GGA Software Services LLC",
                    _indigo.version());
            Window[] windows = JFrame.getOwnerlessWindows();
            msg_box.showMessageDialog(windows[0], ex.getMessage(),
                    "Error loading file", JOptionPane.ERROR_MESSAGE);
         }

         finish_event.fireEvent(null);
      }
   }

   public void start()
   {
      _thread.start();
   }

   public boolean isActive() {
      if (_thread == null)
         return false;
      return _thread.isAlive();
   }

   public boolean isInterrupted() {
      if (_thread == null)
         return false;
      return _thread.isInterrupted();
   }

   public void interrupt() {
      if (_thread == null)
         return;
      _thread.interrupt();
   }
}
