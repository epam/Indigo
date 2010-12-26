/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.gga.indigo.chemdiff;

import com.gga.indigo.Indigo;
import com.gga.indigo.IndigoObject;
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
   Indigo indigo;
   boolean is_save_same;
   ArrayList<CompMol> conc_mols;
   ArrayList<CompMol> uniq_mols1;
   ArrayList<CompMol> uniq_mols2;
   MainFrame main_frame;
   MolComparerThread thread;

   public class CompMol implements Comparable<CompMol>
   {
      IndigoObject molecule;
      String smiles;
      int index;
      int secondary_index;
      boolean is_uniq;

      public CompMol( IndigoObject new_mol, int idx )
      {
         molecule = new_mol;
         index = idx;
         is_uniq = true;
      }

      public CompMol( IndigoObject new_mol, String new_smiles, int idx )
      {
         molecule = new_mol.clone();
         smiles = new_smiles;
         index = idx;
         is_uniq = true;
      }

      public void copy( CompMol another_mol )
      {
         index = another_mol.index;
         secondary_index = another_mol.secondary_index;
         molecule.self = another_mol.molecule.self;
         smiles = new String(another_mol.smiles);
         is_uniq = another_mol.is_uniq;
      }

      public int compareTo( CompMol another_mol )
      {
         return smiles.compareTo(another_mol.smiles);
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

   public MolComparer( MainFrame cur_main_frame )
   {
      try
      {
         String path = MainFrame.getPathToJarfileDir(MainFrame.class);
         if (path == null)
            indigo = new Indigo();
         else
            indigo = new Indigo(path + File.separator + "lib");
         conc_mols = new ArrayList<CompMol>();
         uniq_mols2 = new ArrayList<CompMol>();
         uniq_mols1 = new ArrayList<CompMol>();

         main_frame = cur_main_frame;

         main_frame.getMainProgressBar().setMinimum(0);
         main_frame.getMainProgressBar().setString("");
         main_frame.getMainProgressBar().setStringPainted(true);

         thread = new MolComparerThread();
      } catch (Exception ex) {
         Logger.getLogger(MolComparer.class.getName()).log(Level.SEVERE, null, ex);
      }
   }

   public MolComparer( ArrayList<IndigoObject> mols1, ArrayList<IndigoObject> mols2,
                       MainFrame cur_main_frame ) throws Exception
   {
      main_frame = cur_main_frame;
      main_frame.getMainProgressBar().setMinimum(0);
      main_frame.getMainProgressBar().setString("");
      main_frame.getMainProgressBar().setStringPainted(true);
      setMols(mols1, 0);
      setMols(mols2, 1);
   }

   public void setMols( ArrayList<IndigoObject> mols, int idx ) throws Exception
   {
      ArrayList<CompMol> uniq_mols;

      if (idx == 0)
         uniq_mols = uniq_mols1;
      else
         uniq_mols = uniq_mols2;

      uniq_mols.clear();
      for (int i = 0; i < mols.size(); i++)
         uniq_mols.add(new CompMol(mols.get(i), i));
   }

   class MolComparerThread extends Thread
   {
      int sort_pos;

      private int _buildCSList( int invalid_count, int idx )
      {
         ArrayList<CompMol> uniq_mols;

         if (idx == 0)
            uniq_mols = uniq_mols1;
         else
            uniq_mols = uniq_mols2;
         
         int i = 0;
         while (i < uniq_mols.size())
         {
            try
            {
               IndigoObject mol = uniq_mols.get(i).molecule.clone();
               if (main_frame.getAromatizeCheckState())
                  mol.aromatize();
               if (main_frame.getCisTransCheckState())
                  mol.clearCisTrans();
               if (main_frame.getStereocentersCheckState())
                  mol.clearStereocenters();
               uniq_mols.get(i).smiles = mol.canonicalSmiles().trim();
            }
            catch (Exception ex)
            {
               uniq_mols.get(i).smiles = "unknown #" + invalid_count++;
               main_frame.getMainProgressBar().setMaximum(uniq_mols1.size() + uniq_mols2.size());
            }

            main_frame.getMainProgressBar().setValue(idx * uniq_mols1.size() + i + 1);
            i++;
         }

         return invalid_count;
      }

      private void _calcCanonicalSmiles()
      {
         int i;
         main_frame.getMainProgressBar().setString("Canonical smiles computing");
         main_frame.getMainProgressBar().setMaximum(uniq_mols1.size() + uniq_mols2.size());
         main_frame.getMainProgressBar().setValue(0);

         int invalid_count = 0;

         invalid_count = _buildCSList(invalid_count, 0);
         invalid_count = _buildCSList(invalid_count, 1);
      }
      /*
      public void clearSame()
      {
         for (int i = 0; i < uniq_csmiles1.size(); i++)
            for (int j = 0; j < uniq_csmiles1.size(); j++)
               if (i != j)
                  if (uniq_csmiles1.get(i).smiles.compareTo(
                      uniq_csmiles1.get(j).smiles) == 0)
                  {
                     uniq_csmiles1.remove(j);
                     uniq_mols1.remove(j);
                  }

         for (int i = 0; i < uniq_csmiles2.size(); i++)
            for (int j = 0; j < uniq_csmiles2.size(); j++)
               if (i != j)
                  if (uniq_csmiles2.get(i).smiles.compareTo(
                      uniq_csmiles2.get(j).smiles) == 0)
                  {
                     uniq_csmiles2.remove(j);
                     uniq_mols2.remove(j);
                  }
      }
      */

      private void _compare()
      {
         conc_mols.clear();
         _calcCanonicalSmiles();
         //_sort();
         Collections.sort(uniq_mols1);
         Collections.sort(uniq_mols2);

         main_frame.getMainProgressBar().setString("Molecules comparing");
         main_frame.getMainProgressBar().setMaximum(1000);
         main_frame.getMainProgressBar().setValue(0);

         int init_mols1_count = uniq_mols1.size();
         int init_mols2_count = uniq_mols2.size();

         int pos1 = 0, pos2 = 0;
         while ((pos1 != uniq_mols1.size()) && (pos2 != uniq_mols2.size()))
         {
            int comp_res = uniq_mols1.get(pos1).smiles.compareTo(uniq_mols2.get(pos2).smiles);
            if (comp_res == 0)
            {
               conc_mols.add(uniq_mols1.get(pos1));
               conc_mols.get(conc_mols.size() - 1).secondary_index = uniq_mols2.get(pos2).index;

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
               main_frame.getMainProgressBar().setValue((int)(part1 * 1000));
            else
               main_frame.getMainProgressBar().setValue((int)(part2 * 1000));
         }

         main_frame.getMainProgressBar().setValue(1000);

         return;
      }

      // This method is called when the thread runs
      public void run()
      {
         main_frame.getCompareButton().setEnabled(false);
         _compare();

         Collections.sort(conc_mols, new IndexComparator());
         Collections.sort(uniq_mols1, new IndexComparator());
         Collections.sort(uniq_mols2, new IndexComparator());

         ArrayList<IndigoObject> conc_array = new ArrayList<IndigoObject>();
         ArrayList<Integer> conc_indexes1 = new ArrayList<Integer>();
         ArrayList<Integer> conc_indexes2 = new ArrayList<Integer>();
         for (int i = 0; i < conc_mols.size(); i++)
         {
            conc_array.add(conc_mols.get(i).molecule);
            conc_indexes1.add(conc_mols.get(i).index);
            conc_indexes2.add(conc_mols.get(i).secondary_index);
         }

         ArrayList<IndigoObject> uniq_array1 = new ArrayList<IndigoObject>();
         ArrayList<Integer> uniq_indexes1 = new ArrayList<Integer>();
         for (int i = 0; i < uniq_mols1.size(); i++)
         {
            uniq_array1.add(uniq_mols1.get(i).molecule);
            uniq_indexes1.add(uniq_mols1.get(i).index);
         }

         ArrayList<IndigoObject> uniq_array2 = new ArrayList<IndigoObject>();
         ArrayList<Integer> uniq_indexes2 = new ArrayList<Integer>();
         for (int i = 0; i < uniq_mols2.size(); i++)
         {
            uniq_array2.add(uniq_mols2.get(i).molecule);
            uniq_indexes2.add(uniq_mols2.get(i).index);
         }

         main_frame.getOutputTable(0).setMols(conc_array, conc_indexes1, conc_indexes2);
         main_frame.getOutputTable(1).setMols(uniq_array1, uniq_indexes1, null);
         main_frame.getOutputTable(2).setMols(uniq_array2, uniq_indexes2, null);
         main_frame.getTabPanel().setSelectedIndex(1);
         main_frame.getCompareButton().setEnabled(true);
      }
   }

   public void compare()
   {
      thread.start();
   }
}
