package com.epam.indigo.knime.fremover;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import com.epam.indigo.IndigoObject;

public class IndigoFeatureRemoverUtils {

   public static interface Remover
   {
      public void removeFeature (IndigoObject io);
   }
   
   public static final ArrayList<String> names = new ArrayList<String>();
   public static final Map<String, Remover> removers = new HashMap<String, Remover>();
   
   private static void addRemover (String name, Remover remover)
   {
      removers.put(name, remover);
      names.add(name);
   }
   
   static
   {
      addRemover("Isotopes", new Remover ()
      {
         public void removeFeature (IndigoObject io)
         {
            for (IndigoObject atom : io.iterateAtoms())
               atom.resetIsotope();
         }
      });
      addRemover("Chirality", new Remover ()
      {
         public void removeFeature (IndigoObject io)
         {
            io.clearStereocenters();
            for (IndigoObject bond : io.iterateBonds())
               if (bond.bondOrder() == 1)
                  bond.resetStereo();
         }
      });
      addRemover("Cis-trans", new Remover ()
      {
         public void removeFeature (IndigoObject io)
         {
            io.clearCisTrans();
         }
      });
      addRemover("Highlighting", new Remover ()
      {
         public void removeFeature (IndigoObject io)
         {
            io.unhighlight();
         }
      });
      addRemover("R-sites", new Remover ()
      {
         public void removeFeature (IndigoObject io)
         {
            for (IndigoObject atom : io.iterateAtoms())
               if (atom.isRSite())
                  atom.remove();
         }
      });
      addRemover("Pseudoatoms", new Remover ()
      {
         public void removeFeature (IndigoObject io)
         {
            for (IndigoObject atom : io.iterateAtoms())
               if (atom.isPseudoatom())
                  atom.remove();
         }
      });
      addRemover("Attachment points", new Remover ()
      {
         public void removeFeature (IndigoObject io)
         {
            io.clearAttachmentPoints();
         }
      });
      addRemover("Repeating units", new Remover ()
      {
         public void removeFeature (IndigoObject io)
         {
            for (IndigoObject ru : io.iterateRepeatingUnits())
               ru.remove();
         }
      });
      addRemover("Data S-groups", new Remover ()
      {
         public void removeFeature (IndigoObject io)
         {
            for (IndigoObject sg : io.iterateDataSGroups())
               sg.remove();
         }
      });
      addRemover("Minor components", new Remover ()
      {
         public void removeFeature (IndigoObject io)
         {
            int max_comp = -1, max_comp_size = 0;
            for (IndigoObject comp: io.iterateComponents())
               if (comp.countAtoms() > max_comp_size)
               {
                  max_comp_size = comp.countAtoms();
                  max_comp = comp.index();
               }
            if (max_comp == -1)
               return;
            
            IndigoObject maxComp = io.component(max_comp);
            HashSet<Integer> atomsRemain = new HashSet<Integer>();
            
            for(IndigoObject atom : maxComp.iterateAtoms())
               atomsRemain.add(atom.index());
            
            ArrayList<Integer> indices = new ArrayList<Integer>();
            
            for(IndigoObject atom : io.iterateAtoms())
               if(!atomsRemain.contains(atom.index()))
                  indices.add(atom.index());
            io.removeAtoms(toIntArray(indices));
         }
      });
   }
   
   static int[] toIntArray (List<Integer> list)
   {
      int[] arr = new int[list.size()];
      for (int i = 0; i < list.size(); i++)
         arr[i] = list.get(i).intValue();
      return arr;
   }
   
}
