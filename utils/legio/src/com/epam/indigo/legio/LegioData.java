package com.epam.indigo.legio;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.controls.IndigoCheckedException;
import com.epam.indigo.controls.IndigoObjectWrapper;
import java.io.IOException;
import java.util.ArrayList;

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

   public IndigoObject getReaction ()
   {
       return reaction;
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

   public void setMonomers ( int reatcnt_idx, ArrayList<? extends IndigoObjectWrapper> mols) throws IndigoCheckedException
   {
      monomers_table.at(reatcnt_idx).clear();
      for (IndigoObjectWrapper iterr : mols)
      {
        monomers_table.at(reatcnt_idx).arrayAdd(iterr.getObjectCopy());
      }
   }

   public void setReactionFromFile( String rxn_path )
   {
      if (rxn_path.toLowerCase().endsWith("sma") || rxn_path.toLowerCase().endsWith("smarts"))
          reaction = indigo.loadReactionSmartsFromFile(rxn_path);
      else
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
