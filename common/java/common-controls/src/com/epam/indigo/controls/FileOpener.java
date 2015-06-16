package com.epam.indigo.controls;

import java.io.File;
import java.io.FileFilter;
import javax.swing.JFileChooser;
import javax.swing.JFrame;

public class FileOpener
{
   private JFileChooser _file_chooser = new JFileChooser();
   private File _file = null;

   public FileOpener()
   {
      _file_chooser.setAcceptAllFileFilterUsed(false);
   }

   public void addExtension( String... extensions )
   {
      MolFileFilter mon_ff = new MolFileFilter();
      for (String ext : extensions)
         mon_ff.addExtension(ext);
      _file_chooser.addChoosableFileFilter(mon_ff);
   }

   public String openFile( String approve_button_text )
   {
      _file_chooser.setCurrentDirectory(new File(GlobalParams.getInstance().dir_path));
      _file_chooser.setApproveButtonText(approve_button_text);
      int ret_val = _file_chooser.showDialog(_file_chooser, approve_button_text);
      _file = _file_chooser.getSelectedFile();

      if ((_file == null) || (ret_val != JFileChooser.APPROVE_OPTION))
         return null;

      GlobalParams.getInstance().dir_path = _file.getPath();

      return _file.getPath();
   }

   public File getFile()
   {
      return _file;
   }

   public String getFilePath()
   {
      if (_file == null)
         return null;

      return _file.getPath();
   }

   public MolFileFilter getFileFilter()
   {
      if (_file == null)
         return null;

      return (MolFileFilter)_file_chooser.getFileFilter();
   }
}
