/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import java.io.File;
import java.util.ArrayList;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

/**
 *
 * @author achurinov
 */
public class LoadUtils
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
}
