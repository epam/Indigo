package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoException;
import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.controls.IndigoCheckedException;
import com.ggasoftware.indigo.controls.MoleculeItem;

public class CanonicalCodeGenerator
{
   private volatile CompareOptions _compare_options;
   private Indigo _cached_indigo;
   private IndigoObject _cached_charge_pattern;

   public CanonicalCodeGenerator (CompareOptions compare_options)
   {
      this._compare_options = compare_options;
   }
   
   private IndigoObject getChargePattern (IndigoObject mol)
   {
      if (mol.getIndigo() != _cached_indigo)
      {
         _cached_indigo = mol.getIndigo();
         _cached_charge_pattern = _cached_indigo.loadSmarts(
                 "[#6,#15,#16;+,++,+++]-,=[#6,#7,#8;-,--,---]");
      }
      return _cached_charge_pattern;
   }

   private int _unseparateCharges (IndigoObject mol)
   {
      // At first check if charge configutation exists
      IndigoObject charge_pattern = getChargePattern(mol);
      
      IndigoObject pos_q_atom = charge_pattern.getAtom(0);
      IndigoObject neg_q_atom = charge_pattern.getAtom(1);
      IndigoObject q_bond = charge_pattern.getBond(0);
      
      int cnt = 0, prev_cnt;
      IndigoObject match_iter;
      do
      {
         IndigoObject matcher = mol.getIndigo().substructureMatcher(mol);
         match_iter = matcher.iterateMatches(charge_pattern);
         prev_cnt = cnt;
         for (IndigoObject match: match_iter)
         {
            IndigoObject pos_atom = match.mapAtom(pos_q_atom);
            IndigoObject neg_atom = match.mapAtom(neg_q_atom);
            IndigoObject bond = match.mapBond(q_bond);

            pos_atom.setCharge(pos_atom.charge() - 1);
            neg_atom.setCharge(neg_atom.charge() + 1);
            bond.setBondOrder(bond.bondOrder() + 1);
            cnt++;
         }
      } while (prev_cnt != cnt);
      
      return cnt;
   }

   public IndigoObject createPreparedObject (MoleculeItem object) throws IndigoCheckedException
   {
      Indigo indigo = object.getIndigo();
      synchronized (indigo)
      {
         indigo.setOption("ignore-stereochemistry-errors",
                 _compare_options.getStereocentersIgnoreFlag());

         IndigoObject copy_to_modify = object.getObjectCopy();
         return createClonedPreparedObject(copy_to_modify);
      }
   }
   
   private IndigoObject createClonedPreparedObject (IndigoObject copy_to_modify) throws IndigoCheckedException
   {
      try 
      {
         if (_compare_options.getUnseparateChargesFlag())
            _unseparateCharges(copy_to_modify);
         if (_compare_options.getAromFlag())
            copy_to_modify.aromatize();
         if (_compare_options.getCisTransIgnoreFlag())
            copy_to_modify.clearCisTrans();
         if (_compare_options.getStereocentersIgnoreFlag())
            copy_to_modify.clearStereocenters();
         return copy_to_modify;
      }
      catch (IndigoException ex)
      {
         throw new IndigoCheckedException(ex.getMessage(), ex);
      }
   }
   
   public IndigoObject createPreparedObject (IndigoObject object) throws IndigoCheckedException
   {
      return createClonedPreparedObject(object.clone());
   }
   
   public String generate (MoleculeItem object) throws IndigoCheckedException
   {
      try
      {
         return createPreparedObject(object).canonicalSmiles();
      }
      catch (IndigoException ex)
      {
         throw new IndigoCheckedException(ex.getMessage(), ex);
      }
   }
}
