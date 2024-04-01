package com.epam.indigo.knime.common.components;

import javax.swing.JList;
import javax.swing.JTextField;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

@SuppressWarnings("serial")
public class FilteredList<T> extends JList<T> {
   /**
    * Filtered list view class
    */
   
   private FilteredField input;
   
   public FilteredList(FilteredListModel<T> model) {
      super(model);
      input = new FilteredField(model);
   }
   
   public void addItem(T item) {
      ((FilteredListModel<T>)getModel()).addItem(item);
   }
   
   public FilteredField getFilteredField() {
      return input;
   }
   
   class FilteredField extends JTextField implements DocumentListener {
      /**
       * Search field class
       */
      
      FilteredListModel<T> model;
      
      public FilteredField(FilteredListModel<T> model) {
         super();
         this.model = model;
         getDocument().addDocumentListener(this);
      }
      
      private void updateList() {
         FilteredList.this.clearSelection();
         model.setInput(this.getText());
         model.filter();
      }
      
      @Override
      public void changedUpdate(DocumentEvent e) {
         updateList();
      }
      
      @Override
      public void insertUpdate(DocumentEvent e) {
         updateList();
      }
      
      @Override
      public void removeUpdate(DocumentEvent e) {
         updateList();
      } 
   }
}
