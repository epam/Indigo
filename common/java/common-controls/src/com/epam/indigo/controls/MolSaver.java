package com.epam.indigo.controls;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import java.io.File;
import java.util.ArrayList;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

public class MolSaver
{
   private Indigo _indigo;
   private FileOpener _fopener = new FileOpener();
   private ArrayList<RenderableObject> _failed_to_save = null;
   
   private boolean saveReactionProducts = false;

   public MolSaver (Indigo indigo)
   {
      this._indigo = indigo;
   }

   public void addExtension (String... extensions)
   {
      _fopener.addExtension(extensions);
   }
   
   public ArrayList<RenderableObject> getInvalidObjects ()
   {
      return _failed_to_save;
   }

   void setSaveReactionProducts (boolean state)
   {
       saveReactionProducts = state;
   }
   
   public String saveMols (ArrayList<? extends RenderableObject> mol_datas)
   {
      _failed_to_save = new ArrayList<RenderableObject>();
      
      IndigoObject output_file = null;
      try
      {
         String out_file_path = _fopener.openFile("Save");
         if (out_file_path == null)
            return null;
         MolFileFilter cur_filter = _fopener.getFileFilter();
         String choosed_extension = cur_filter.getDefaultExtension();
         if (!cur_filter.accept(_fopener.getFile()))
            out_file_path += "." + choosed_extension;

         if (choosed_extension.compareTo("mol") == 0)
         {
            if (mol_datas.isEmpty() && mol_datas.size() > 1)
               throw new IndigoCheckedException("Can save only single molecules into MOL format");
            mol_datas.get(0).getRenderableObject().saveMolfile(out_file_path);
            return out_file_path;
         }

         if (choosed_extension.compareTo("rxn") == 0)
         {
            if (mol_datas.isEmpty() && mol_datas.size() > 1)
               throw new IndigoCheckedException("Can save only single item into RXN format");
            mol_datas.get(0).getRenderableObject().saveRxnfile(out_file_path);
            return out_file_path;
         }

         IndigoObject file_saver = null;
         try
         {
            file_saver = _indigo.createFileSaver(out_file_path, choosed_extension);

            for (RenderableObject mol : mol_datas)
            {
               try 
               {
                  IndigoObject m = mol.getRenderableObject();
                  if (m == null)
                  {
                     _failed_to_save.add(mol);
                     continue;
                  }
                  if (!m.hasCoord())
                     m.layout();
                  m.markEitherCisTrans();

                  if (!saveReactionProducts)
                      file_saver.append(m);
                  else
                      for (IndigoObject product: m.iterateProducts())
                          file_saver.append(product);
               }
               catch (IndigoException ex)
               {
                  _failed_to_save.add(mol);
               }
            }
         }
         finally
         {
            if (file_saver != null)
               file_saver.close();
         }

         return _fopener.getFilePath();
      }
      catch (Exception ex)
      {
         JOptionPane.showMessageDialog(JFrame.getOwnerlessWindows()[0],
                 ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
         return null;
      }
      finally
      {
         if (output_file != null)
            output_file.close();
      }
   }

   public String saveMol (RenderableObject mol)
   {
      ArrayList<RenderableObject> mols = new ArrayList<RenderableObject>();
      mols.add(mol);

      return saveMols(mols);
   }
}
