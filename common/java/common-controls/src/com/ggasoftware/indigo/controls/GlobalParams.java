/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.controls;

import java.io.File;

public class GlobalParams
{
   public String dir_path;
   public boolean is_in = false;

   private volatile static GlobalParams _instance;
   
   private GlobalParams() 
   {
      dir_path = (new File("")).getAbsolutePath();
   }
   
   public static GlobalParams getInstance()
   {
      if (_instance == null)
      {
         synchronized (GlobalParams.class)
         {
            if (_instance == null)
               _instance = new GlobalParams();
         }
      }
      
      return _instance;
   }
}
