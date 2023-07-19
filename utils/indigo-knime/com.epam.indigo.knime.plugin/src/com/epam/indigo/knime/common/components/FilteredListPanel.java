package com.epam.indigo.knime.common.components;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.ListCellRenderer;

import com.epam.indigo.knime.common.IndigoListPanel;

@SuppressWarnings("serial")
public class FilteredListPanel<T> extends IndigoListPanel<T>{

   public FilteredListPanel(FilteredListModel<T> model, ListCellRenderer<T> cellRenderer) {
      
      super(new FilteredList<T>(model), cellRenderer);
   
      FilteredList<T> list = ((FilteredList<T>) this.getList());
      
      JTextField searchField = list.getFilteredField();
      JScrollPane avScroller = new JScrollPane(list);
      avScroller.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

      searchField.setToolTipText("Search");
    
      // fill panel up
      GridBagLayout gbl = new GridBagLayout();
      gbl.columnWidths = new int[]{0, 0};
      gbl.rowHeights = new int[]{0, 0, 0};
      gbl.columnWeights = new double[]{1.0, Double.MIN_VALUE};
      gbl.rowWeights = new double[]{0.0, 1.0, Double.MIN_VALUE};
      this.setLayout(gbl);
       
      GridBagConstraints gbcSearch = new GridBagConstraints();
      gbcSearch.insets = new Insets(0, 0, 5, 0);
      gbcSearch.fill = GridBagConstraints.HORIZONTAL;
      gbcSearch.gridx = 0;
      gbcSearch.gridy = 0;
      this.add(searchField, gbcSearch);
      searchField.setColumns(20);
       
      GridBagConstraints gbcList = new GridBagConstraints();
      gbcList.fill = GridBagConstraints.BOTH;
      gbcList.gridx = 0;
      gbcList.gridy = 1;
      this.add(avScroller, gbcList);
   }
   
}
