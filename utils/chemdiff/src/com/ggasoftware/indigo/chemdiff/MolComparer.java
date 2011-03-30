/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.controls.*;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.Integer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.Collection;

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
   IndigoEventSource<Integer> finish_event = new IndigoEventSource<Integer>(null);
   IndigoEventSource<Integer> progress_event = new IndigoEventSource<Integer>(this);
   CanonicalCodeGenerator csmiles_generator;
   private static int _invalid_count = 0;
   FileWriter out_file[] = new FileWriter[2];

   public class CompMol implements Comparable<CompMol>
   {
      ArrayList<Integer> primary_indices;
      ArrayList<Integer> secondary_indices;
      RenderableMolData mol_data;
      String csmiles = null;
      boolean is_uniq;

      public CompMol( RenderableMolData mol_data, int index )
      {
         this.mol_data = mol_data;
         primary_indices = new ArrayList<Integer>();
         primary_indices.add(index);
         secondary_indices = new ArrayList<Integer>();
         is_uniq = true;
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
         if (mol1.primary_indices.get(0) > mol2.primary_indices.get(0))
            return 1;
         else if (mol1.primary_indices.get(0) < mol2.primary_indices.get(0))
            return -1;
         else
            return 0;
      }
   }

   public MolComparer( CanonicalCodeGenerator csmiles_generator, boolean is_save_same )
   {
      try
      {
         out_file[0] = new FileWriter("log1.txt");
         out_file[1] = new FileWriter("log2.txt");

         this.is_save_same = is_save_same;
         this.csmiles_generator = csmiles_generator;
         conc_mols = new ArrayList<CompMol>();
         uniq_mols2 = new ArrayList<CompMol>();
         uniq_mols1 = new ArrayList<CompMol>();

         finish_event = new IndigoEventSource<Integer>(null);

         thread = new MolComparerThread();
      } catch (Exception ex) {
         Logger.getLogger(MolComparer.class.getName()).log(Level.SEVERE, null, ex);
      }
   }

   public ArrayList< ArrayList<Integer> > getIdxArrays( boolean is_conc, int set_idx )
   {
       ArrayList< ArrayList<Integer> > indexes_arrays = new ArrayList< ArrayList<Integer> >();
       ArrayList<CompMol> comp_mol_array = null;
       boolean is_primary_indexes = true;
       if (is_conc)
       {
          comp_mol_array = conc_mols;
          is_primary_indexes = (set_idx == 0 ? true : false);
       }
       else
          comp_mol_array = (set_idx == 0 ? uniq_mols1 : uniq_mols2);

       for (CompMol comp_mol : comp_mol_array)
       {
          ArrayList<Integer> indexes_array_i = new ArrayList<Integer>();
          indexes_array_i.addAll(is_primary_indexes ? comp_mol.primary_indices :
                                                    comp_mol.secondary_indices);
          indexes_arrays.add(indexes_array_i);
       }
       return indexes_arrays;
   }

   public void setMols(ArrayList<RenderableMolData> mol_datas, int idx ) throws Exception
   {
      ArrayList<CompMol> uniq_mols = (idx == 0 ? uniq_mols1 : uniq_mols2);

      uniq_mols.clear();
      int i = 0;
      for (RenderableMolData mol_data : mol_datas)
         uniq_mols.add(new CompMol(mol_data, i++));
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
               uniq_mols.get(pos).primary_indices.add(uniq_mols.get(pos + 1).primary_indices.get(0));
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

            int progress_level;
            if (part1 > part2)
               progress_level = (int)(part2 * 1000);
            else
               progress_level = (int)(part1 * 1000);
            
            progress_event.fireEvent(progress_level);
         }

         progress_event.fireEvent(1000);

         return;
      }

      private void _writeDebugInfo( int idx, int i, String smiles )
      {
         try {
            out_file[idx].append("" + i + ": " + smiles + "\n");
         } catch (IOException ex) {
            Logger.getLogger(MolComparer.class.getName()).log(Level.SEVERE, null, ex);
         } 
      }

      private synchronized void _buildCSmiles( int idx )
      {
         ArrayList<CompMol> uniq_mols = (idx == 0 ? uniq_mols1 : uniq_mols2);

         int i = 0;
         for (CompMol comp_mol : uniq_mols)
         {
            progress_event.fireEvent((int)(((float)i++ / uniq_mols.size()) * 1000));
            String failed_name = new String() + "invalid molecule #" + _invalid_count++;
            if (comp_mol.mol_data != null)
            {
               try {
                  comp_mol.csmiles = csmiles_generator.generate(comp_mol.mol_data);
               } catch (Exception ex) {
                  comp_mol.csmiles = failed_name;
               }
            }
            else
               comp_mol.csmiles = failed_name;
            
            _writeDebugInfo(idx, i, comp_mol.csmiles);

            if ((i % 10000) == 0)
               System.gc();
         }
      }

      // This method is called when the thread runs
      public void run()
      {
         finish_event.fireEvent(0);
         _buildCSmiles(0);
         finish_event.fireEvent(1);
         _buildCSmiles(1);
         finish_event.fireEvent(2);

         _compare();

         Collections.sort(conc_mols, new IndexComparator());
         Collections.sort(uniq_mols1, new IndexComparator());
         Collections.sort(uniq_mols2, new IndexComparator());

         finish_event.fireEvent(3);

         try {
            out_file[0].close();
            out_file[1].close();
         } catch (IOException ex) {
            Logger.getLogger(MolComparer.class.getName()).log(Level.SEVERE, null, ex);
         }

      }
   }

   public void compare()
   {
      thread.start();
   }
}
