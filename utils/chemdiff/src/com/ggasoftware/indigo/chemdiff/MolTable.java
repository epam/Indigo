/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.controls.MolData;
import java.util.ArrayList;

public interface MolTable {
   public abstract void setMols(ArrayList<? extends MolData> mol_datas,
              ArrayList< ArrayList<Integer> > indexes1,
              ArrayList< ArrayList<Integer> > indexes2);

   public abstract void clear();
}
