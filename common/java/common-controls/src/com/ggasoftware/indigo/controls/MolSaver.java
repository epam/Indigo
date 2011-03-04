package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import java.io.File;
import java.util.ArrayList;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

public class MolSaver {
   Indigo indigo;

   public MolSaver( Indigo indigo )
   {
      this.indigo = indigo;
   }

   public String saveMols(ArrayList<MolData> mol_datas) {
      IndigoObject output_file = null;
      try {
         JFileChooser file_chooser = new JFileChooser();

         MolFileFilter sdf_ff = new MolFileFilter();
         sdf_ff.addExtension("sdf");
         sdf_ff.addExtension("sd");

         MolFileFilter smi_ff = new MolFileFilter();
         smi_ff.addExtension("smi");

         MolFileFilter cml_ff = new MolFileFilter();
         cml_ff.addExtension("cml");

         file_chooser.setAcceptAllFileFilterUsed(false);
         file_chooser.addChoosableFileFilter(sdf_ff);
         file_chooser.addChoosableFileFilter(smi_ff);
         file_chooser.addChoosableFileFilter(cml_ff);
         file_chooser.setFileFilter(sdf_ff);

         file_chooser.setCurrentDirectory(new File(GlobalParams.getInstance().dir_path));
         int ret_val = file_chooser.showSaveDialog(JFrame.getOwnerlessWindows()[0]);
         File choosed_file = file_chooser.getSelectedFile();

         if ((choosed_file == null) || (ret_val != JFileChooser.APPROVE_OPTION)) {
            return null;
         }
         GlobalParams.getInstance().dir_path = choosed_file.getParent();

         String out_file_path = choosed_file.getPath();
         MolFileFilter cur_filter = (MolFileFilter)file_chooser.getFileFilter();
         if (!cur_filter.accept(choosed_file))
            out_file_path += "." + cur_filter.getDefaultExtension();

         output_file = indigo.writeFile(out_file_path);
         if (cur_filter.getDefaultExtension().compareTo("cml") == 0)
            output_file.cmlHeader();

         for (int i = 0; i < mol_datas.size(); i++) {
            MolData mol = mol_datas.get(i);
            IndigoObject m = mol.mol_iterator.at(mol.index).clone();
            if (m == null) {
               continue;
            }
            if (!m.hasCoord())
               m.layout();
            m.markEitherCisTrans();
            /*
            String[] orig_id_strs = (String[]) table.getValueAt(i, 0);
            for (int j = 0; j < orig_id_strs.length; j++)
               m.setProperty(String.format("original_id%d", j + 1), orig_id_strs[j]);
            */
            if (cur_filter.getDefaultExtension().compareTo("sdf") == 0)
               output_file.sdfAppend(m);
            else if (cur_filter.getDefaultExtension().compareTo("smi") == 0)
               output_file.smilesAppend(m);
            else if (cur_filter.getDefaultExtension().compareTo("cml") == 0)
               output_file.cmlAppend(m);
            else
               throw new Exception("Unknown extension");
         }

         if (cur_filter.getDefaultExtension().compareTo("cml") == 0)
            output_file.cmlFooter();

         return choosed_file.getPath();
      } catch (Exception ex) {
         JOptionPane.showMessageDialog(JFrame.getOwnerlessWindows()[0],
                 ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
         return null;
      } finally {
         if (output_file != null)
            output_file.close();
      }
   }

   public String saveMol(MolData mol) {
      try {
         JFileChooser file_chooser = new JFileChooser();

         MolFileFilter mol_ff = new MolFileFilter();
         mol_ff.addExtension("mol");

         file_chooser.setAcceptAllFileFilterUsed(false);
         file_chooser.addChoosableFileFilter(mol_ff);

         file_chooser.setCurrentDirectory(new File(GlobalParams.getInstance().dir_path));
         int ret_val = file_chooser.showSaveDialog(JFrame.getOwnerlessWindows()[0]);
         File choosed_file = file_chooser.getSelectedFile();

         if ((choosed_file == null) || (ret_val != JFileChooser.APPROVE_OPTION)) {
            return null;
         }

         GlobalParams.getInstance().dir_path = choosed_file.getParent();

         String out_file_path = choosed_file.getPath();
         MolFileFilter cur_filter = (MolFileFilter)file_chooser.getFileFilter();
         if (!cur_filter.accept(choosed_file))
            out_file_path += "." + cur_filter.getDefaultExtension();

         IndigoObject m = mol.mol_iterator.at(mol.index).clone();
         if (m == null) {
            return null;
         }
         if (!m.hasCoord())
            m.layout();
         m.markEitherCisTrans();

         if (cur_filter.getDefaultExtension().compareTo("mol") == 0)
            m.saveMolfile(out_file_path);
         else
            throw new Exception("Unknown extension");

         return choosed_file.getPath();
      } catch (Exception ex) {
         JOptionPane.showMessageDialog(JFrame.getOwnerlessWindows()[0],
                 ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
         return null;
      }
   }
}
