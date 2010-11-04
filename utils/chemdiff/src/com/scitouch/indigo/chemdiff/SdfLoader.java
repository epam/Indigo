/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.scitouch.indigo.chemdiff;

import com.scitouch.indigo.Indigo;
import com.scitouch.indigo.IndigoObject;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.nio.CharBuffer;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JButton;
import javax.swing.JProgressBar;
import javax.swing.JTable;

/**
 *
 * @author achurinov
 */
public class SdfLoader
{
   private Indigo indigo;
   private File file;
   private ArrayList<IndigoObject> molecules;
   private SdfLoadRunnable runnable;
   private Thread thread;
   private boolean is_sdf;
   private MainFrame main_frame;
   private int table_idx;

   public SdfLoader ( Indigo cur_indigo, MainFrame new_main_frame, int cur_table_idx, boolean is_sdf_flag )
   {
      main_frame = new_main_frame;
      table_idx = cur_table_idx;

      indigo = cur_indigo;
      molecules = new ArrayList<IndigoObject>();
      runnable = new SdfLoadRunnable();
      thread = new Thread(null, runnable, "sdf_loader #" + table_idx, 10000000);
      is_sdf = is_sdf_flag;
   }

   public void setFile ( File cur_file )
   {
      file = cur_file;
   }

   class SdfLoadRunnable implements Runnable
   {
      // This method is called when the thread runs
      public void run()
      {
            main_frame.getInputTable(table_idx).setVisible(false);
            main_frame.getLoadButton(table_idx).setEnabled(false);
            main_frame.getCompareButton().setEnabled(false);

            int file_pos = 0;
            int old_cnt = 0;
            long old_time = 0;

            IndigoObject iterator_object;

            if (is_sdf)
               iterator_object = indigo.iterateSDFile(file.getPath());
            else
               iterator_object = indigo.iterateSmilesFile(file.getPath());

            for (IndigoObject iterr : iterator_object)
            {
               file_pos = iterator_object.tell();

               try
               {
                  molecules.add(iterr.clone());
               }
               catch ( Exception ex )
               {
                  molecules.add(null);
               }

               main_frame.getLoadProgressBar(table_idx).setValue(file_pos);
            }

            main_frame.getInputTable(table_idx).setMols(molecules);
            main_frame.getLoadButton(table_idx).setEnabled(true);
            if (!main_frame.getInputTable((table_idx + 1) % 2).getSdfLoader().isActive())
               main_frame.getCompareButton().setEnabled(true);
            main_frame.getInputTable(table_idx).setVisible(true);
      }
   }

   public void start ()
   {
      thread.start();
   }

   public void interrupt ()
   {
      thread.interrupt();
   }

   public boolean isActive ()
   {
      return thread.isAlive();
   }
}
