/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.gui.ProgressEvent;
import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.gui.*;
import com.ggasoftware.indigo.IndigoObject;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.event.ListSelectionEvent;
import java.io.*;

/**
 *
 * @author achurinov
 */
public class MolComparer
{
   boolean is_save_same;
   ArrayList<CompMol> conc_mols;
   ArrayList<CompMol> uniq_mols1;
   ArrayList<CompMol> uniq_mols2;
   MolComparerThread thread;
   CompareFinishEvent finish_event;
   ProgressEvent progress_event;

   public class CompMol extends MolData implements Comparable<CompMol>
   {
      ArrayList<Integer> primary_indices;
      ArrayList<Integer> secondary_indices;
      boolean is_uniq;

      public CompMol( MolData mol_data )
      {
         if (mol_data != null)
         {
            mol_iterator = mol_data.mol_iterator;
            index = mol_data.index;
            if (mol_data.csmiles != null)
               csmiles = new String(mol_data.csmiles);
            else
               csmiles = null;
         }

         primary_indices = new ArrayList<Integer>();
         primary_indices.add(index);
         secondary_indices = new ArrayList<Integer>();
         is_uniq = true;
      }

      public void copy( CompMol another_mol )
      {
         primary_indices.addAll(another_mol.secondary_indices);
         secondary_indices.addAll(another_mol.secondary_indices);

         mol_iterator = another_mol.mol_iterator;
         index = another_mol.index;
         if (another_mol.csmiles != null)
            csmiles = new String(another_mol.csmiles);
         else
            csmiles = null;


         is_uniq = another_mol.is_uniq;
      }

      public int compareTo( CompMol another_mol )
      {
         if (csmiles == null)
            return 0;
         if (another_mol.csmiles == null)
            return 1;
         return csmiles.compareTo(another_mol.csmiles);
      }
   }

   public class IndexComparator implements Comparator<CompMol>
   {
      public int compare(CompMol mol1, CompMol mol2)
      {
         if (mol1.index > mol2.index)
            return 1;
         else if (mol1.index < mol2.index)
            return -1;
         else
            return 0;
      }
   }

   public MolComparer( boolean is_save_same )
   {
      try
      {
         this.is_save_same = is_save_same;
         String path = MainFrame.getPathToJarfileDir(MainFrame.class);
/*
         if (path == null)
            indigo = new Indigo();
         else
            indigo = new Indigo(path + File.separator + "lib");
*/
         conc_mols = new ArrayList<CompMol>();
         uniq_mols2 = new ArrayList<CompMol>();
         uniq_mols1 = new ArrayList<CompMol>();

         finish_event = new CompareFinishEvent();
         progress_event = new ProgressEvent(-1, 0);

         thread = new MolComparerThread();
      } catch (Exception ex) {
         Logger.getLogger(MolComparer.class.getName()).log(Level.SEVERE, null, ex);
      }
   }

   public void setMols( ArrayList<MolData> mol_datas, int idx ) throws Exception
   {
      ArrayList<CompMol> uniq_mols;

      if (idx == 0)
         uniq_mols = uniq_mols1;
      else
         uniq_mols = uniq_mols2;

      uniq_mols.clear();
      for (int i = 0; i < mol_datas.size(); i++)
         uniq_mols.add(new CompMol(mol_datas.get(i)));
   }

   class MolComparerThread extends Thread
   {
      //Only for sorted arrays
      private void _clearSame(int idx)
      {
         ArrayList<CompMol> uniq_mols = (idx == 0 ? uniq_mols1 : uniq_mols2);
         int pos = 0;

         while (pos < uniq_mols.size())
         {
            while (pos + 1 < uniq_mols.size() &&
                   uniq_mols.get(pos + 1).compareTo(uniq_mols.get(pos)) == 0)
            {
               uniq_mols.get(pos).primary_indices.add(uniq_mols.get(pos + 1).index);
               uniq_mols.remove(pos + 1);
            }
            pos++;
         }
      }

      private void _compare()
      {
         conc_mols.clear();

         Collections.sort(uniq_mols1);
         Collections.sort(uniq_mols2);

         if (!is_save_same)
         {
            _clearSame(0);
            _clearSame(1);
         }
         int init_mols1_count = uniq_mols1.size();
         int init_mols2_count = uniq_mols2.size();

         int pos1 = 0, pos2 = 0;
         while ((pos1 != uniq_mols1.size()) && (pos2 != uniq_mols2.size()))
         {
            int comp_res = uniq_mols1.get(pos1).csmiles.compareTo(
                           uniq_mols2.get(pos2).csmiles);
            if (comp_res == 0)
            {
               conc_mols.add(uniq_mols1.get(pos1));
               conc_mols.get(conc_mols.size() - 1).secondary_indices.addAll(
                       uniq_mols2.get(pos2).primary_indices);

               uniq_mols1.remove(pos1);
               uniq_mols2.remove(pos2);
            }
            else if (comp_res < 0)
               pos1++;
            else
               pos2++;

            double part1 = (double)(init_mols1_count - uniq_mols1.size() - pos1) / init_mols1_count;
            double part2 = (double)(init_mols2_count - uniq_mols2.size() - pos2) / init_mols2_count;

            if (part1 > part2)
               progress_event.progress = (int)(part1 * 1000);
            else
               progress_event.progress = (int)(part1 * 1000);

            progress_event.alertListeners();
         }

         progress_event.progress = 1000;
         progress_event.alertListeners();

         return;
      }

      // This method is called when the thread runs
      public void run()
      {
         _compare();

         Collections.sort(conc_mols, new IndexComparator());
         Collections.sort(uniq_mols1, new IndexComparator());
         Collections.sort(uniq_mols2, new IndexComparator());

         finish_event.alertListeners();
      }
   }

   public void compare()
   {
      thread.start();
   }
}
