/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.scitouch.indigo.chemdiff;

import java.io.File;

/**
 *
 * @author achurinov
 */
public class CurDir
{
   public String dir_path;

   public CurDir()
   {
      dir_path = new File("").getAbsolutePath();
   }
}
