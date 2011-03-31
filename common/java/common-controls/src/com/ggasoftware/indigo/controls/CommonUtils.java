/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import javax.swing.ImageIcon;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

/**
 *
 * @author achurinov
 */
public class CommonUtils
{
   public static IndigoObject getIterator( Indigo indigo, String path) throws Exception
   {
      IndigoObject iterator;

      String ext="";
      int point_idx = path.lastIndexOf(".");
      ext = path.substring(point_idx + 1, path.length());

      if (ext.equals("sdf") || ext.equals("sd") || ext.equals("mol")) {
         iterator = indigo.iterateSDFile(path);
      } else if (ext.equals("smi")) {
         iterator = indigo.iterateSmilesFile(path);
      } else if (ext.equals("cml")) {
         iterator = indigo.iterateCMLFile(path);
      } else {
         throw new Exception("Unsupported file extension");
      }

      return iterator;
   }
   
   public static void showAboutDialog( JFrame parent )
   {
       String msg = String.format("ChemDiff\nVersion %s\nCopyright (C) 2010-2011 GGA Software Services LLC",
               (new Indigo()).version());
       JOptionPane.showConfirmDialog(parent, msg, "About", JOptionPane.DEFAULT_OPTION,
               JOptionPane.INFORMATION_MESSAGE,
               new ImageIcon("images\\logo_small.png"));
   }
}
