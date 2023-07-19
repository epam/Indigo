package com.epam.indigo.knime.common;

import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.ListCellRenderer;


/**
 * Class describes a panel with a list on it as a member of {@link com.epam.indigo.knime.common.IndigoItemsFilterPanel}.
 */
@SuppressWarnings("serial")
public class IndigoListPanel<T>  extends JPanel {

   private JList<T> list;
   
   public IndigoListPanel(JList<T> list, ListCellRenderer<T> cellRenderer) {
      list.setCellRenderer(cellRenderer);
      this.setList(list);
   }
   
   public JList<T> getList() {
      return list;
   }
   
   private void setList(JList<T> list) {
      this.list = list;
   }
}
