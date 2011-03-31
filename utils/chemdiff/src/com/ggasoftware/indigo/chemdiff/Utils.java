/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.chemdiff;

import java.util.ArrayList;

/**
 *
 * @author achurinov
 */
public class Utils {
   public static synchronized String[] makeIdxString(ArrayList<Integer> indexes) {
      String[] idx_strings = new String[indexes.size()];

      for (int i = 0; i < indexes.size(); i++)
         idx_strings[i] = (new String()) + indexes.get(i);

      return idx_strings;
   }
}
