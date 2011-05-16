/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.controls;

import java.io.File;

public class GlobalParams
{
   // TODO: save config and options
   public String dir_path;

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
