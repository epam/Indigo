package com.gga.indigo.legio;

import com.gga.indigo.Indigo;
import com.gga.indigo.IndigoObject;
import java.io.FileWriter;
import java.io.IOException;
import javax.swing.JOptionPane;

public class LegioData
{
   IndigoObject reaction;
   IndigoObject monomers_table;
   private IndigoObject output_reactions;
   Indigo indigo;

   public LegioData( Indigo cur_indigo )
   {
      indigo = cur_indigo;
      monomers_table = indigo.createArray();
      output_reactions = indigo.createArray();
   }

   public void clearReactantMonomers( int reactant_idx )
   {
      IndigoObject reactant_monomers = monomers_table.at(reactant_idx);
      reactant_monomers.clear();
   }

   public int getProductsCount()
   {
      return output_reactions.count();
   }

   public IndigoObject getOutReaction( int idx )
   {
      if (idx >=  output_reactions.count())
         return null;

      return output_reactions.at(idx);
   }

   public IndigoObject getOutProduct( int idx )
   {
      if (idx >=  output_reactions.count())
         return null;

      IndigoObject rxn = output_reactions.at(idx);

      for (IndigoObject iterr : rxn.iterateProducts())
      {
         return iterr;
      }

      return null;
   }

   public String getOutReactionString( int idx )
   {
      if (idx >=  output_reactions.count())
         return null;

      return output_reactions.at(idx).rxnfile();
   }

   public String getOutProductString( int idx )
   {
      if (idx >=  output_reactions.count())
         return null;

      IndigoObject rxn = output_reactions.at(idx);

      for (IndigoObject iterr : rxn.iterateProducts())
      {
         return iterr.molfile();
      }

      return null;
   }

   public int getMonomersCount( int reactant_idx )
   {
      return monomers_table.at(reactant_idx).count();
   }

   public IndigoObject getMonomer( int reactant_idx, int mon_idx )
   {
      return monomers_table.at(reactant_idx).at(mon_idx);
   }

   public String getMonomerString( int reactant_idx, int mon_idx )
   {
      if (reactant_idx >= monomers_table.count())
         return null;
      if (mon_idx >= monomers_table.at(reactant_idx).count())
         return null;

      return monomers_table.at(reactant_idx).at(mon_idx).molfile();
   }

   public void addMonomerFromFile( int reatcnt_idx, String mon_path )
   {
      IndigoObject mons_iterator = indigo.iterateSDFile(mon_path);
      for (IndigoObject iterr : mons_iterator)
      {
         try
         {
            monomers_table.at(reatcnt_idx).arrayAdd(iterr.clone());
         } catch (Exception ex)
         {
            int i;
            i = 1;
         }
      }
   }

   public void setReactionFromFile( String rxn_path )
   {
      reaction = indigo.loadQueryReactionFromFile(rxn_path);

      for (int i = 0; i < reaction.countReactants(); i++)
         monomers_table.arrayAdd(indigo.createArray());
   }

   public int getReactantsCount()
   {
      if (reaction == null)
         return 0;

      return reaction.countReactants();
   }

   public void clear()
   {
      monomers_table.clear();
      output_reactions.clear();
   }

   public void react() throws IOException
   {
      output_reactions = indigo.reactionProductEnumerate(reaction, monomers_table);

      for ( int i = 0; i < output_reactions.count(); i++)
      {
         for (IndigoObject iterr : output_reactions.at(i).iterateReactants())
         {
            iterr.layout();
         }
         for (IndigoObject iterr : output_reactions.at(i).iterateProducts())
         {
            iterr.layout();
         }
      }
   }
}
