package com.gga.indigo.legio;

import com.gga.indigo.Indigo;
import com.gga.indigo.IndigoObject;

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
      IndigoObject reactant_monomers = monomers_table.arrayAt(reactant_idx);
      reactant_monomers.arrayClear();
   }

   public int getProductsCount()
   {
      return output_reactions.arrayCount();
   }

   public IndigoObject getOutReaction( int idx )
   {
      return output_reactions.arrayAt(idx);
   }

   public IndigoObject getOutProduct( int idx )
   {
      return output_reactions.arrayAt(idx).iterateProducts();
   }

   public String getOutReactionString( int idx )
   {
      if (idx >=  output_reactions.arrayCount())
         return null;

      return output_reactions.arrayAt(idx).rxnfile();
   }

   public String getOutProductString( int idx )
   {
      if (idx >=  output_reactions.arrayCount())
         return null;

      IndigoObject rxn = output_reactions.arrayAt(idx);

      for (IndigoObject iterr : rxn.iterateProducts())
      {
         return iterr.molfile();
      }

      return null;
   }

   public int getMonomersCount( int reactant_idx )
   {
      return monomers_table.arrayAt(reactant_idx).arrayCount();
   }

   public IndigoObject getMonomer( int reactant_idx, int mon_idx )
   {
      return monomers_table.arrayAt(reactant_idx).arrayAt(mon_idx);
   }

   public String getMonomerString( int reactant_idx, int mon_idx )
   {
      if (reactant_idx >= monomers_table.arrayCount())
         return null;
      if (mon_idx >= monomers_table.arrayAt(reactant_idx).arrayCount())
         return null;

      return monomers_table.arrayAt(reactant_idx).arrayAt(mon_idx).molfile();
   }

   public void addMonomerFromFile( int reatcnt_idx, String mon_path )
   {
      for (IndigoObject iterr : indigo.iterateSDFile(mon_path))
      {
         monomers_table.arrayAt(reatcnt_idx).arrayAdd(iterr.clone());
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
      monomers_table.arrayClear();
      output_reactions.arrayClear();
   }

   public void react()
   {
      output_reactions = indigo.reactionProductEnumerate(reaction, monomers_table);

      for ( int i = 0; i < output_reactions.arrayCount(); i++)
      {
         for (IndigoObject iterr : output_reactions.arrayAt(i).iterateProducts())
         {
            iterr.layout();
         }
      }
   }
}
