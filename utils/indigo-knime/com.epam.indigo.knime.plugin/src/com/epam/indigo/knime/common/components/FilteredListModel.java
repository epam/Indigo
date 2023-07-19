package com.epam.indigo.knime.common.components;

import java.util.ArrayList;

import com.epam.indigo.knime.common.IndigoListModel;
import com.epam.indigo.knime.common.IndigoNodeSettings;

@SuppressWarnings("serial")
abstract public class FilteredListModel<T> extends IndigoListModel<T> {
   /**
    * List model class
    */
   
   ArrayList<T> allItems;
   ArrayList<T> filteredItems;
   String input;
   
   public FilteredListModel() {
      this.allItems = new ArrayList<T>();
      this.filteredItems = new ArrayList<T>();
      setInput("");
   }
   
   public void setInput(String text) {
      input = text;
   }
   
   @Override
   public T getElementAt(int ind) {
      return (ind < filteredItems.size()) ? filteredItems.get(ind) : null;
   }
   
   @Override
   public int getSize() {
      return filteredItems.size();
   }
   
   @Override
   public void addItem(T item) {
      allItems.add(item);
      filter();
   }
   
   @Override
   public void removeItem(T item) {
      allItems.remove(item);
      filter();
   }
   
   @Override
   public void removeAllItems() {
      allItems.removeAll(filteredItems);
      filter();
   }
   
   public boolean containItem(T item) {
      return allItems.contains(item);
   }
   
   public void filter() {
      filteredItems.clear();
      for (int i = 0; i < allItems.size(); i++) {
         String lowItem = allItems.get(i).toString().toLowerCase();
         String lowInput = input.toLowerCase();
         int pos = lowItem.indexOf(lowInput);
         if (pos != -1) {
            filteredItems.add(allItems.get(i));
         }
      }
      fireContentsChanged(this, 0, getSize());
   }

   @Override
   abstract public void saveSettingsTo(IndigoNodeSettings settings);

   @Override
   abstract public void loadSettingsFrom(IndigoNodeSettings settings);
}


