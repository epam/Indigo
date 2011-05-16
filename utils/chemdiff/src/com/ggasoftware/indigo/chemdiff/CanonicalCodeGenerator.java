package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoException;
import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.controls.IndigoCheckedException;

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
                 "[#6,#7,#8,#15,#16;+]-,=[#6,#7,#8,#15,#16;-]");
      }
      return _cached_charge_pattern;
   }

   private int _unseparateCharges (IndigoObject mol)
   {
      // At first check if charge configutation exists
      IndigoObject matcher = mol.getIndigo().substructureMatcher(mol);
      if (matcher.match(getChargePattern(mol)) == null)
         return 0;

      int cnt = 0, old_cnt;
      do
      {
         old_cnt = cnt;
         IndigoObject bonds_iterator = mol.iterateBonds();

         for (IndigoObject bond : bonds_iterator)
         {
            IndigoObject source = bond.source();
            IndigoObject destination = bond.destination();

            if (source.degree() > 1 && destination.degree() > 1)
            {
               continue;
            }

            int order = bond.bondOrder();

            if (order != 1 && order != 2)
            {
               continue;
            }

            int elem_beg = source.atomicNumber();
            int elem_end = destination.atomicNumber();

            // C,N,O,P,S
            if (elem_beg != 6 && elem_beg != 7 && elem_beg != 8 && elem_beg != 15 && elem_beg != 16)
               continue;
            if (elem_end != 6 && elem_end != 7 && elem_end != 8 && elem_end != 15 && elem_end != 16)
               continue;

            int charge_beg = source.charge();
            int charge_end = destination.charge();

            if (charge_beg > 0 && charge_end < 0)
            {
               source.setCharge(charge_beg - 1);
               destination.setCharge(charge_end + 1);
               bond.setBondOrder(order + 1);
               cnt++;
               break;
            }
            else if (charge_beg < 0 && charge_end > 0)
            {
               source.setCharge(charge_beg + 1);
               destination.setCharge(charge_end - 1);
               bond.setBondOrder(order + 1);
               cnt++;
               break;
            }
         }
      } while (cnt != old_cnt);

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
