package com.epam.indigo.knime.common;

import javax.swing.AbstractListModel;

@SuppressWarnings("serial")
public abstract class IndigoListModel<T> extends AbstractListModel<T>  {
   /**
    *  Class to provide common behavior to list' models
    */
 
   abstract public void addItem(T item);
   abstract public void removeItem(T item);
   abstract public void removeAllItems();
   
   /**
    * Method to save data from list model to settings object if exists.
    * Required to be implemented at least for the right list.
    */
   abstract public void saveSettingsTo(IndigoNodeSettings settings);
   
   /**
    * Method to load data to list model from settings object if exists
    * Required to be implemented for the both left and right lists.
    */
   abstract public void loadSettingsFrom(IndigoNodeSettings settings);
   
}
