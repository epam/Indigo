package com.epam.indigo.uploader;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.util.Iterator;
import java.util.Properties;
import java.util.Scanner;


public class SdfIterator implements Iterable<SdfIterator.SDItem> {

   static SDItem createItem(String str) {
      return new SDItem(str);
   }
   private final Scanner _molScanner;
   private  String _nextElement;

   public Iterable<String> delimIter() {
      return () -> {
         _readNextElement();
         return new Iterator<String>() {

            @Override
            public boolean hasNext() {
               return (_nextElement != null);
            }

            @Override
            public String next() {
               String result = _nextElement;
               _readNextElement();
               return result;
            }

            @Override
            public void remove() {
               throw new UnsupportedOperationException();
            }
         };
      };
   }

   public static class SDItem {
      public String mol = "";
      public final Properties props = new Properties();
      public static final String  SD_PROPERTIES = "\\Q\n>\\E.*\\Q<\\E";
      public byte[] buf;
      public Object jsonObject;

      private SDItem(String mol) {
         try (Scanner pScanner = new Scanner(new ByteArrayInputStream(mol.getBytes()))) {
            pScanner.useDelimiter(SD_PROPERTIES);
            /*
             * Read molfile
             */
            if (pScanner.hasNext()) {
               this.mol = pScanner.next();
            }
            /*
             * Read properties
            */
            while (pScanner.hasNext()) {
               String propList = pScanner.next().trim();
               String p_name, p_val = "";

               if(propList.length() > 0) {
                  int key_idx = propList.indexOf("\n");
                  if(key_idx == -1) {
                     key_idx = propList.length();
                  }
                  p_name = propList.substring(0, key_idx);
                  int k_idx = p_name.lastIndexOf(">");
                  if(k_idx > 0 ) {
                     p_name = p_name.substring(0, k_idx);
                  }

                  if(key_idx < propList.length() - 1) {
                     p_val = propList.substring(key_idx+1);
                  }
                  props.setProperty(p_name, p_val);
               }
            }
      }
      }
   }

   public SdfIterator(InputStream str) {
      _molScanner = new Scanner(new BufferedInputStream(str));
      _molScanner.useDelimiter(java.util.regex.Pattern.quote("$$$$") + "((\\r\\n)|(\\n))");
   }
   void _readNextElement() {
      _nextElement = null;
      if(_molScanner.hasNext()) {
         String mol = _molScanner.next();
         if(mol.trim().length() > 0) {
            _nextElement = mol;
         }
      }
   }
   @Override
   public Iterator<SdfIterator.SDItem> iterator() {
      _readNextElement();
      return new Iterator<SdfIterator.SDItem>() {
         @Override
         public boolean hasNext() {
            return (_nextElement != null);
         }
         @Override
         public SDItem next() {
            String result = _nextElement;
            _readNextElement();
            if(result != null)
               return new SDItem(result);
            return null;
         }

         @Override
         public void remove() {
            throw new UnsupportedOperationException();
         }
      };
   }
}
