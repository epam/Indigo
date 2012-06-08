package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.controls.MessageBox;
import java.io.PrintWriter;
import java.io.StringWriter;
import javax.swing.UIManager;

public class Main
{
   public static void main (String[] args) throws InterruptedException
   {
      try
      {
         /*
          * Enumerate look and feel and set it for testing
         LookAndFeelInfo[] installedLookAndFeels = UIManager.getInstalledLookAndFeels();
         for (LookAndFeelInfo info: installedLookAndFeels)
            System.out.println(info.getClassName());
          */
         //UIManager.setLookAndFeel("com.sun.java.swing.plaf.nimbus.NimbusLookAndFeel");
         UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
      }
      catch (Exception e)
      {
         System.out.println("Error setting native LAF: " + e);
      }
      
      try
      {
         MainFrame mf = new MainFrame();
         mf.setLocationRelativeTo(null);
         mf.setVisible(true);
      } 
      catch (Throwable err)
      {
         StringWriter sw = new StringWriter();
         err.printStackTrace(new PrintWriter(sw));
         String error_as_string = sw.toString();

         MessageBox.show(null, error_as_string, "Error", MessageBox.ICON_ERROR);
         System.exit(0);
      }
   }
}
