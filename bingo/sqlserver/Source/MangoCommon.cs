using System.Collections.Generic;

namespace indigo
{
   public class MoleculeHashElement
   {
      public int count;
      public int hash;
   }
   public class MoleculeHash
   {
      public List<MoleculeHashElement> elements = new List<MoleculeHashElement>();
   }
}
