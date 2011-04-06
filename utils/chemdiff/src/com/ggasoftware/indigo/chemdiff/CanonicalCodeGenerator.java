package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import com.sun.jna.Callback;

public class CanonicalCodeGenerator implements Callback {

   private volatile CompareOptions _compare_options;
   private Indigo _indigo;
   private IndigoObject _charge_patten;

   public CanonicalCodeGenerator(Indigo indigo, CompareOptions compare_options)
   {
      this._compare_options = compare_options;
   }

   private int _unseparateCharges (IndigoObject mol)
   {
      // At first check if charge configutation exists
      IndigoObject matcher = _indigo.substructureMatcher(mol);
      if (matcher.match(_charge_patten) == null)
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
      _charge_patten = indigo.loadSmarts("[#6,#7,#8,#15,#16;+]-,=[#6,#7,#8,#15,#16;-]");
   }

   public synchronized String generate( Object object )
   {
      RenderableMolData mol_data = (RenderableMolData)object;
      IndigoObject mol_iterator = mol_data.getIterator();
      int index = mol_data.getIndex();

      synchronized (_indigo) {
         IndigoObject mol = mol_iterator.at(index);

         if (_compare_options.getUnseparateChargesFlag())
            _unseparateCharges(mol);
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
