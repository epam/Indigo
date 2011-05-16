/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import java.net.URL;
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
   public static IndigoObject getIterator (Indigo indigo, String path)
   {
      IndigoObject iterator;

      if (path.endsWith(".sdf") || path.endsWith(".sd") || path.endsWith(".mol"))
         iterator = indigo.iterateSDFile(path);
      else if (path.endsWith(".rdf"))
         iterator = indigo.iterateRDFile(path);
      else if (path.endsWith(".smi"))
         iterator = indigo.iterateSmilesFile(path);
      else if (path.endsWith(".cml"))
         iterator = indigo.iterateCMLFile(path);
      else
         throw new RuntimeException("Unsupported file extension");

      return iterator;
   }

   public static void showAboutDialog (JFrame parent, String product, String url)
   {
      StringBuilder sb = new StringBuilder();
      sb.append(String.format("<b>%s</b><br>", product));
      sb.append(String.format("Indigo version: %s<br>", (new Indigo()).version()));
      sb.append("(C) 2010-2011 GGA Software Services LLC<br>");
      sb.append(String.format("<a href=\"%s\">%s</a>", url, url));
      
      URL icon_url = CommonUtils.class.getResource("images/gga-logo.png");
      MessageBox.showHtml(parent, sb.toString(), "About", new ImageIcon(icon_url));
   }
}
