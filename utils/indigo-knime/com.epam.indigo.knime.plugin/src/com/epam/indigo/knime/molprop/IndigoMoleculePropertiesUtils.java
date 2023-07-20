package com.epam.indigo.knime.molprop;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataColumnSpecCreator;
import org.knime.core.data.DataType;
import org.knime.core.data.def.DoubleCell;
import org.knime.core.data.def.IntCell;
import org.knime.core.data.def.StringCell;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;

public class IndigoMoleculePropertiesUtils {

   public static interface PropertyCalculator
   {
      DataType type ();
      DataCell calculate (IndigoObject io, String... atoms);
   }
   
   public static abstract class PropertyCalc implements PropertyCalculator {
      private String name;
      
      public String getName() {
         return name;
      }

      public void setName(String name) {
         this.name = name;
      }
      
      @Override
      public String toString() {
         return getName();
      }
   }

   public static abstract class IntPropertyCalculator extends PropertyCalc {
      
      
      public IntPropertyCalculator(String name) {
         setName(name);
      }

      public DataType type ()
      {
         return IntCell.TYPE;
      }


   }

   public static abstract class DoublePropertyCalculator  extends PropertyCalc {
      
      public DoublePropertyCalculator(String name) {
         setName(name);
      }

      public DataType type ()
      {
         return DoubleCell.TYPE;
      }

   }

   public static abstract class StringPropertyCalculator  extends PropertyCalc {
      
      public StringPropertyCalculator(String name) {
         setName(name);
      }

      public DataType type ()
      {
         return StringCell.TYPE;
      }

   }

   public static final ArrayList<String> names = new ArrayList<String>();
   public static final Map<String, PropertyCalc> calculators = new HashMap<String, PropertyCalc>();
   public static final Map<String, DataColumnSpec> colSpecs = new HashMap<String, DataColumnSpec>();
   public static DataColumnSpec[] colSpecsArray;  

   private static void putCalculator (String name, PropertyCalc pc)
   {
      calculators.put(name, pc);
      names.add(name);
   }
   
   static
   {
      putCalculator("Molecular formula", new StringPropertyCalculator("Molecular formula")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new StringCell(io.grossFormula());
         }
      });
      
      putCalculator("Molecular weight", new DoublePropertyCalculator("Molecular weight")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new DoubleCell(io.molecularWeight());
         }
      });

      putCalculator("Most abundant mass", new DoublePropertyCalculator("Most abundant mass")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new DoubleCell(io.mostAbundantMass());
         }
      });

      putCalculator("Monoisotopic mass", new DoublePropertyCalculator("Monoisotopic mass")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new DoubleCell(io.monoisotopicMass());
         }
      });

      putCalculator("Number of connected components", new IntPropertyCalculator("Number of connected components")
            {
               public DataCell calculate (IndigoObject io, String... atoms)
               {
                  return new IntCell(io.countComponents());
               }
            });

      
      putCalculator("Total number of atoms", new IntPropertyCalculator("Total number of atoms")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new IntCell(io.countHeavyAtoms() + io.countHydrogens());
         }
      });

      putCalculator("Number of hydrogens", new IntPropertyCalculator ("Number of hydrogens")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new IntCell(io.countHydrogens());
         }
      });
      
      putCalculator("Number of heavy atoms", new IntPropertyCalculator("Number of heavy atoms")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new IntCell(io.countHeavyAtoms());
         }
      });

      putCalculator("Number of heteroatoms", new IntPropertyCalculator("Number of heteroatoms")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            int count = 0;
            
            for (IndigoObject atom : io.iterateAtoms())
            {
               if (atom.isRSite() || atom.isPseudoatom())
                  continue;
               
               int number = atom.atomicNumber();
               
               if (number > 1 && number != 6)
                  count++;
            }
               
            return new IntCell(count);
         }
      });
      

      putCalculator("Number of carbons", new IntPropertyCalculator("Number of carbons")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            int count = 0;
            
            for (IndigoObject atom : io.iterateAtoms())
            {
               if (atom.isRSite() || atom.isPseudoatom())
                  continue;
               
               int number = atom.atomicNumber();
               
               if (number == 6)
                  count++;
            }
               
            return new IntCell(count);
         }
      });      
      
      putCalculator("Number of aromatic atoms", new IntPropertyCalculator("Number of aromatic atoms")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            int count = 0;
            io.aromatize();
            for (IndigoObject atom : io.iterateAtoms())
               for (IndigoObject nei : atom.iterateNeighbors())
                  if (nei.bond().bondOrder() == 4)
                  {
                     count++;
                     break;
                  }
            return new IntCell(count);
         }
      });

      putCalculator("Number of aliphatic atoms", new IntPropertyCalculator("Number of aliphatic atoms")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            int count = 0;
            io.aromatize();
            for (IndigoObject atom : io.iterateAtoms())
            {
               boolean aromatic = false;
               
               for (IndigoObject nei : atom.iterateNeighbors())
                  if (nei.bond().bondOrder() == 4)
                  {
                     aromatic = true;
                     break;
                  }
               if (!aromatic)
                  count++;
            }
            return new IntCell(count);
         }
      });
      
      putCalculator("Number of pseudoatoms", new IntPropertyCalculator("Number of pseudoatoms")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new IntCell(io.countPseudoatoms());
         }
      });

      
      putCalculator("Number of visible atoms", new IntPropertyCalculator("Number of visible atoms")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new IntCell(io.countAtoms());
         }
      });

      putCalculator("Number of chiral centers", new IntPropertyCalculator("Number of chiral centers")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new IntCell(io.countStereocenters());
         }
      });
      
      putCalculator("Number of R-sites", new IntPropertyCalculator("Number of R-sites")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new IntCell(io.countRSites());
         }
      });
      
      putCalculator("Number of bonds", new IntPropertyCalculator("Number of bonds")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            return new IntCell(io.countBonds());
         }
      });

      putCalculator("Number of aromatic bonds", new IntPropertyCalculator("Number of aromatic bonds")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            int count = 0;
            io.aromatize();
            for (IndigoObject bond : io.iterateBonds())
               if (bond.bondOrder() == 4)
                  count++;
            return new IntCell(count);
         }
      });

      putCalculator("Number of aliphatic bonds", new IntPropertyCalculator("Number of aliphatic bonds")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            int count = 0;
            io.aromatize();
            for (IndigoObject bond : io.iterateBonds())
               if (bond.bondOrder() != 4)
                  count++;
            return new IntCell(count);
         }
      });

      putCalculator("Number of cis/trans bonds", new IntPropertyCalculator("Number of cis/trans bonds")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            int count = 0;

            for (IndigoObject bond : io.iterateBonds())
            {
               int stereo = bond.bondStereo();
               if (stereo == Indigo.CIS || stereo == Indigo.TRANS)
                  count++;
            }
            return new IntCell(count);
         }
      });
      
      putCalculator("Number of aromatic rings", new IntPropertyCalculator("Number of aromatic rings")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            int count = 0;
            io.aromatize();
            
            for (IndigoObject ring : io.iterateSSSR())
            {
               boolean arom = true;
               
               for (IndigoObject bond : ring.iterateBonds())
                  if (bond.bondOrder() != 4)
                     arom = false;
               
               if (arom)
                  count++;
            }
            
            return new IntCell(count);
         }
      });
      
      putCalculator("Number of aliphatic rings", new IntPropertyCalculator("Number of aliphatic rings")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            int count = 0;
            io.aromatize();
            
            for (IndigoObject ring : io.iterateSSSR())
            {
               for (IndigoObject bond : ring.iterateBonds())
                  if (bond.bondOrder() != 4)
                  {
                     count++;
                     break;
                  }
            }
            
            return new IntCell(count);
         }
      });
    
      putCalculator("Number of heteroatoms in aromatic rings", new IntPropertyCalculator("Number of heteroatoms in aromatic rings")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            IndigoObject matcher = io.getIndigo().substructureMatcher(io);
            int count = matcher.countMatches(io.getIndigo().loadSmarts("[!#6;a;R]"));
            return new IntCell(count);
         }
      });
      
      putCalculator("Number of heteroatoms in aliphatic rings", new IntPropertyCalculator("Number of heteroatoms in aliphatic rings")
      {
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            IndigoObject matcher = io.getIndigo().substructureMatcher(io);
            int count = matcher.countMatches(io.getIndigo().loadSmarts("[!#6;A;R]"));
            return new IntCell(count);
         }
      });

      putCalculator("Highlighted atoms", new StringPropertyCalculator("Highlighted atoms") {
         public DataCell calculate(IndigoObject io, String... atoms) {
            StringBuilder highlighted = new StringBuilder();

            for (IndigoObject atom : io.iterateAtoms()) {
               if (atom.isHighlighted()) {
                  if (highlighted.length() != 0)
                     highlighted.append(',');
                  highlighted.append(atom.index() + 1);
               }
            }
            return new StringCell(highlighted.toString());
         }
      });

      putCalculator("Highlighted bonds", new StringPropertyCalculator("Highlighted bonds") {
         public DataCell calculate(IndigoObject io, String... atoms) {
            StringBuilder highlighted = new StringBuilder();

            for (IndigoObject bond : io.iterateBonds()) {
               if (bond.isHighlighted()) {
                  if (highlighted.length() != 0)
                     highlighted.append(',');
                  highlighted.append(bond.index() + 1);
               }
            }
            return new StringCell(highlighted.toString());
         }
      });

      putCalculator("User-specified atoms count", new IntPropertyCalculator("User-specified atoms count") 
      {
         
         
         public DataCell calculate (IndigoObject io, String... atoms)
         {
            int count = 0; // count of atoms in the molecule contained in the atoms argument
            Map<String, Integer> atomsTable  = grossFormulaToMap(io.grossFormula());
            if (atomsTable.size() > 0 && atoms.length > 0) {
               Set<String> atomsSet = atomsTable.keySet(); 
               for (String atom: atoms){
                  if (atomsSet.contains(atom))
                     count += atomsTable.get(atom);
               }
            }
            return new IntCell(count);
         }
         
         /**
          * @param grossFormula is gross formula of a molecule
          * @return map of pairs <atom name, atom number> for the molecule
          */
         private Map<String, Integer> grossFormulaToMap(String grossFormula) {
            HashMap<String, Integer> map = new HashMap<String, Integer>();
            String[] atomNumPairs = grossFormula.replaceAll("\\s", "").split("(?<=((\\d)|(\\p{L})))(?=\\p{Lu})");
            for (String atomNumPair : atomNumPairs) {
               String[] atomAndNum = atomNumPair.split("(?<=\\p{L})(?=\\d)");
               map.put(atomAndNum[0], (atomAndNum.length > 1 ? Integer.parseInt(atomAndNum[1]) : 1));
            }  
            return map;
         }
         
      });

      colSpecsArray = new DataColumnSpec[names.size()];
      
      for (int i = 0; i < names.size(); i++)
      {
         String key = names.get(i);
         DataType type = ((PropertyCalculator) calculators.get(key)).type();
         DataColumnSpec spec = new DataColumnSpecCreator(key, type).createSpec(); 
         colSpecs.put(key, spec);
         colSpecsArray[i] = spec;
      }
   }
   
}
