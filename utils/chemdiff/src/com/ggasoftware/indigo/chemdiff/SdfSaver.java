/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.chemdiff;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author achurinov
 */
public class SdfSaver
{
   FileWriter fwriter;

   public SdfSaver( String file_path ) throws IOException
   {
      fwriter = null;
      try
      {
         fwriter = new FileWriter(file_path);
      } catch (FileNotFoundException ex) {
         Logger.getLogger(SdfLoader.class.getName()).log(Level.SEVERE, null, ex);
      }
   }

   public SdfSaver( File file ) throws IOException
   {
      fwriter = null;
      try
      {
         fwriter = new FileWriter(file);
      } catch (FileNotFoundException ex) {
         Logger.getLogger(SdfLoader.class.getName()).log(Level.SEVERE, null, ex);
      }
   }

   public void save( ArrayList<String> mol_strings ) throws IOException
   {
      for (int i = 0; i < mol_strings.size(); i++)
      {
         fwriter.write(mol_strings.get(i));
         fwriter.write("> <original_id>");
         fwriter.write("" + 0);

         fwriter.write("$$$$\n");
      }

      fwriter.close();
   }
}
