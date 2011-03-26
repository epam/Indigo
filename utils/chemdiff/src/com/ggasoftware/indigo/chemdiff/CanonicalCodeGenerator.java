package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.controls.MolData;
import com.sun.jna.Callback;

public class CanonicalCodeGenerator implements Callback {

   private volatile CompareOptions _compare_options;
   private Indigo _indigo;

   public CanonicalCodeGenerator(Indigo indigo, CompareOptions compare_options)
   {
      this._compare_options = compare_options;
      this._indigo = indigo;
   }

   private int _unseparateCharges (IndigoObject mol)
   {
      synchronized (_indigo) {
         int cnt = 0, old_cnt;

         do
         {
            old_cnt = cnt;
            IndigoObject bonds_iterator = mol.iterateBonds();

            for (IndigoObject bond : bonds_iterator)
            {
               if (bond.source().degree() > 1 && bond.destination().degree() < 1)
                  continue;

               int order = bond.bondOrder();

               if (order != 1 && order != 2)
                  continue;

               int elem_beg = bond.source().atomicNumber();
               int elem_end = bond.destination().atomicNumber();

               // C,N,O,P,S
               if (elem_beg != 6 && elem_beg != 7 && elem_beg != 8 && elem_beg != 15 && elem_beg != 16)
                  continue;
               if (elem_end != 6 && elem_end != 7 && elem_end != 8 && elem_end != 15 && elem_end != 16)
                  continue;

               int charge_beg = bond.source().charge();
               int charge_end = bond.destination().charge();

               if (charge_beg > 0 && charge_end < 0)
               {
                  bond.source().setCharge(charge_beg - 1);
                  bond.destination().setCharge(charge_end + 1);
                  bond.setBondOrder(order + 1);
                  cnt++;
                  break;
               }
               else if (charge_beg < 0 && charge_end > 0)
               {
                  bond.source().setCharge(charge_beg + 1);
                  bond.destination().setCharge(charge_end - 1);
                  bond.setBondOrder(order + 1);
                  cnt++;
                  break;
               }
            }
         } while (cnt != old_cnt);

         return cnt;
      }
   }

   static private int _unseparateCharges2 (IndigoObject mol)
   {
      int cnt = 0, old_cnt;

      do
      {
         old_cnt = cnt;
         IndigoObject bonds_iterator = mol.iterateBonds();

         for (IndigoObject bond : bonds_iterator)
         {
            IndigoObject source = bond.source();
            IndigoObject destination = bond.destination();

            if (source.degree() > 1 && destination.degree() < 1)
               continue;

            int order = bond.bondOrder();

            if (order != 1 && order != 2)
               continue;

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

   public void setIndigo( Indigo indigo )
   {
      this._indigo = indigo;
   }

   public synchronized String generate( Object object )
   {
      MolData mol_data = (MolData)object;
      IndigoObject mol_iterator = mol_data.getIterator();
      int index = mol_data.getIndex();

      synchronized (_indigo) {
         IndigoObject mol = mol_iterator.at(index);

         if (_compare_options.getUnseparateChargesFlag())
            _unseparateCharges2(mol);
         if (_compare_options.getAromFlag())
            mol.aromatize();
         if (_compare_options.getCisTransIgnoreFlag())
            mol.clearCisTrans();
         if (_compare_options.getStereocentersIgnoreFlag())
            mol.clearStereocenters();
         
         String smiles = mol.canonicalSmiles();

         return smiles;
      }
   }
}
