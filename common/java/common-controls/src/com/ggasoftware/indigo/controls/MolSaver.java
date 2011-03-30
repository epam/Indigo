package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import java.io.File;
import java.util.ArrayList;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

public class MolSaver {
   private Indigo _indigo;
   private FileOpener _fopener = new FileOpener();

   public MolSaver( Indigo indigo )
   {
      this._indigo = indigo;
   }

   public void addExtension( String... extensions )
   {
      _fopener.addExtension(extensions);
   }

   public String saveMols(ArrayList<? extends RenderableObject> mol_datas) {
      IndigoObject output_file = null;
      try {
         String out_file_path = _fopener.openFile();
         if (out_file_path == null)
            return null;
         MolFileFilter cur_filter = _fopener.getFileFilter();
         String choosed_extension = cur_filter.getDefaultExtension();
         if (!cur_filter.accept(_fopener.getFile()))
            out_file_path += "." + choosed_extension;

         output_file = _indigo.writeFile(out_file_path);

         IndigoObject file_saver = _indigo.createSaver(output_file, choosed_extension);

         for (RenderableObject mol : mol_datas) {
            IndigoObject m = mol.getObject();
            if (m == null) {
               continue;
            }
            if (!m.hasCoord())
               m.layout();
            m.markEitherCisTrans();


            file_saver.append(m);
         }

         return _fopener.getFilePath();
      } catch (Exception ex) {
         JOptionPane.showMessageDialog(JFrame.getOwnerlessWindows()[0],
                 ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
         return null;
      } finally {
         if (output_file != null)
            output_file.close();
      }
   }

   public String saveMol(RenderableObject mol) {
      ArrayList<RenderableObject> mols = new ArrayList<RenderableObject>();
      mols.add(mol);

      return saveMols(mols);
   }
}
