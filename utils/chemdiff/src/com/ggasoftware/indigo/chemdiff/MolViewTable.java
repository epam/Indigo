package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.*;
import com.ggasoftware.indigo.gui.MolEvent;
import com.ggasoftware.indigo.gui.MolEventListener;
import com.ggasoftware.indigo.gui.MolSaver;
import com.ggasoftware.indigo.gui.MolTable;
import java.io.File;
import java.util.EventListener;
import java.util.EventObject;
import javax.swing.*;

public class MolViewTable extends MolTable
{
   public SdfLoader sdf_loader;
   private int table_idx;
   public boolean arom_flag;
   public boolean cistrans_flag;
   public boolean stereocenters_ignore_flag;

   /* Default constructor */
   public MolViewTable(Indigo cur_indigo, IndigoRenderer cur_indigo_renderer, MolSaver cur_mol_saver,
              int cur_table_idx,
              int cur_cell_w, int cur_cell_h, boolean is_reactions)
   {
      table_idx = cur_table_idx;

      init(cur_indigo, cur_indigo_renderer, cur_mol_saver, cur_cell_w, cur_cell_h, is_reactions);

      sdf_loader = null;
   }

   public SdfLoader getSdfLoader()
   {
      return sdf_loader;
   }

   public String openSdf(File choosed_file, EventListener finish_event_listener, 
                     EventListener progress_event_listener, boolean arom_flag,
                     boolean cistrans_flag, boolean stereocenters_ignore_flag)
   {
      try {
         sdf_loader = new SdfLoader(indigo, table_idx, null);

         sdf_loader.finish_event.addListener((MolEventListener)finish_event_listener);
         sdf_loader.progress_event.addListener((MolEventListener)progress_event_listener);

         this.arom_flag = arom_flag;
         this.cistrans_flag = cistrans_flag;
         this.stereocenters_ignore_flag = stereocenters_ignore_flag;


         clear();
         mol_datas.clear();

         // Set option for ignoring stereocenter errors
         if (stereocenters_ignore_flag)
            indigo.setOption("ignore-stereochemistry-errors", "1");
         else
            indigo.setOption("ignore-stereochemistry-errors", "0");

         String file_name = choosed_file.getPath().toLowerCase();
         if (file_name.endsWith(".smi")) {
            sdf_loader.setExtension("smi");
         } else if (file_name.endsWith(".sdf") || file_name.endsWith(".sd")) {
            sdf_loader.setExtension("sdf");
         } else if (file_name.endsWith(".cml")) {
            sdf_loader.setExtension("cml");
         } else {
            throw new Exception("Unsupported file extension");
         }

         //sdf_loader.finish_event.addListener(new SdfLoadEventListener());

         sdf_loader.setFile(choosed_file);
         sdf_loader.start(arom_flag, cistrans_flag, stereocenters_ignore_flag);

         return choosed_file.getPath();
      } catch (Exception ex) {
         JOptionPane msg_box = new JOptionPane();
         msg_box.showMessageDialog((JFrame) (getTopLevelAncestor()), ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);

         System.err.println(">>>>" + ex.getMessage());
         ex.printStackTrace();

         return null;
      }
   }

   public String saveSdf(CurDir cur_dir) {
      return mol_saver.saveMols(cur_dir, mol_datas);
   }
}
