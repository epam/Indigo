package com.epam.indigo.knime.isomers;

public class GrayCodes {
   private final int indices[];  

   private static final int START = -1;
   private static final int END = -2;
   
   private int bitChangeIndex;
   
   public GrayCodes (int length)
   {
      indices = new int[length + 1];
      for (int i = 0; i <= length; i++)
         indices[i] = i; 
      
      bitChangeIndex = length > 0 ? START : END;
   }
   
   public boolean isDone ()
   {
      return bitChangeIndex == END;
   }
   
   public void next ()
   {
      bitChangeIndex = indices[0];
      if (bitChangeIndex == indices.length - 1)
         bitChangeIndex = END;
      else 
      {
         indices[0] = 0;   
         indices[bitChangeIndex] = indices[bitChangeIndex + 1];
         indices[bitChangeIndex + 1] = bitChangeIndex + 1;
      }
   }
   
   public int getBitChangeIndex ()
   {
      return bitChangeIndex;
   }
}
