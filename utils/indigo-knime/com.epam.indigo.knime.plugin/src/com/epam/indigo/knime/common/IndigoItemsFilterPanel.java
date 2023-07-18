package com.epam.indigo.knime.common;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;


/**
 * This panel contains three other panels: two of them contains custom lists {@link com.epam.indigo.knime.common.IndigoListPanel}
 * and one of them is a control panel including buttons to move items between the lists.
 */
@SuppressWarnings("serial")
public class IndigoItemsFilterPanel<T> extends JPanel {

   // sizes
   private final int SIDE_PANEL_WIDTH = 240;
   private final int CENTRAL_PANEL_WIDTH = 140;
   private final int SIDE_PANEL_HEIGHT = 240;
   private final Dimension SIDE_PANEL_SIZE = new Dimension(SIDE_PANEL_WIDTH, SIDE_PANEL_HEIGHT);
   private final Dimension CENTRAL_PANEL_SIZE = new Dimension(CENTRAL_PANEL_WIDTH, SIDE_PANEL_HEIGHT);
   
   // the panels
   private ControlPanel ctrlPanel;
   private IndigoListPanel<T> leftPanel;
   private IndigoListPanel<T> rightPanel;
   
   /**
    * @param lPanel is left panel containing custom lists
    * @param rPanel is right panel containing custom lists
    */
   public IndigoItemsFilterPanel(IndigoListPanel<T> lPanel, IndigoListPanel<T> rPanel) {
      
      // initialize panels
      leftPanel = lPanel;
      rightPanel = rPanel;
      ctrlPanel = new ControlPanel(leftPanel.getList(), rightPanel.getList());

      // set preferred panels sizes
      leftPanel.setPreferredSize(SIDE_PANEL_SIZE);
      rightPanel.setPreferredSize(SIDE_PANEL_SIZE);
      ctrlPanel.setPreferredSize(CENTRAL_PANEL_SIZE);

      // box to set the panels
      Box base = Box.createHorizontalBox();
      
      // place panels
      base.add(leftPanel);
      base.add(ctrlPanel);
      base.add(rightPanel);
      
      // set panels' borders
      leftPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.RED), "Excluded"));
      ctrlPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.GRAY), "Select"));
      rightPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.GREEN), "Included"));
      
      // define double click action to the panels' lists
      rightPanel.getList().addMouseListener(new MouseAdapter() {
         @Override
         public void mouseClicked(final MouseEvent me) {
            if (me.getClickCount() == 2)
               moveSelectedOptions(rightPanel.getList(), leftPanel.getList());
         }
      });

      leftPanel.getList().addMouseListener(new MouseAdapter() {
         @Override
         public void mouseClicked(final MouseEvent me) {
            if (me.getClickCount() == 2)
               moveSelectedOptions(leftPanel.getList(), rightPanel.getList());
         }
      });
      
      this.setLayout(new BorderLayout());
      this.add(base);
   }
   
   private class ControlPanel extends JPanel {
      /**
       * The panel contains buttons to move items between two lists.
       */
      
      // sizes
      private final int BUTTON_WIDTH = 120;
      private final int BUTTON_HEIGHT = 20;
      private final int SPACE = 20;
      private final Dimension BUTTON_SIZE = new Dimension(BUTTON_WIDTH, BUTTON_HEIGHT);
      private final Dimension CTRL_PANEL_SIZE = new Dimension(BUTTON_WIDTH + SPACE, SIDE_PANEL_HEIGHT);
      
      // buttons to control lists
      private final JButton btnAdd;
      private final JButton btnAddAll;
      private final JButton btnRemove;
      private final JButton btnRemoveAll;
      
      public ControlPanel(final JList<T> left, final JList<T> right) {

         // set control panel size
         this.setMaximumSize(CTRL_PANEL_SIZE);
         this.setMinimumSize(CTRL_PANEL_SIZE);
         
         // buttons to move items
         btnAdd = new JButton("Add >");
         btnAddAll = new JButton("Add all >>");
         btnRemove = new JButton("< Remove");
         btnRemoveAll = new JButton("<< Remove all");

         // create vertical container
         Box btnBox = Box.createVerticalBox();
         
         // add selected items button
         btnAdd.setEnabled(false); // as nothing selected at init state
         btnAdd.setMaximumSize(BUTTON_SIZE);
         btnAdd.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(final ActionEvent arg0) {
               moveSelectedOptions(left, right);
            }
         });
         
         // add all items button
         btnAddAll.setEnabled(left.getModel().getSize() > 0);
         btnAddAll.setMaximumSize(BUTTON_SIZE);
         btnAddAll.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(final ActionEvent arg0) {
               moveAllOptions(left, right);
            }
         });
         
         // remove selected items button
         btnRemove.setEnabled(false); // as nothing selected at init state
         btnRemove.setMaximumSize(BUTTON_SIZE);
         btnRemove.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(final ActionEvent arg0) {
               moveSelectedOptions(right, left);
            }
         });
         
         // remove all items button
         btnRemoveAll.setEnabled(right.getModel().getSize() > 0);
         btnRemoveAll.setMaximumSize(BUTTON_SIZE);
         btnRemoveAll.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(final ActionEvent arg0) {
               moveAllOptions(right, left);
            }
         });
         
         // Add listener which disables button depending on selected items
         left.addListSelectionListener(new ListSelectionListener() {
     
            private int selectedCount() {
               return left.getSelectedValuesList().size();
            }
     
            @Override
            public void valueChanged(ListSelectionEvent arg0) {
               btnAdd.setEnabled(selectedCount() > 0);
            }
         });
         
         // Add listener to control btnAddAll enabling
         left.getModel().addListDataListener(new ListDataListener() {
         
          private void update() {
             if (left.getModel().getSize() == 0) {
                btnAddAll.setEnabled(false);
             } else {
                btnAddAll.setEnabled(true);
             }
          }
         
          @Override
          public void intervalRemoved(ListDataEvent arg0) {
             update();
          }
         
          @Override
          public void intervalAdded(ListDataEvent arg0) {
             update();
          }
         
          @Override
          public void contentsChanged(ListDataEvent arg0) {
             update();
          }
         });
         
         // Add listener which disables buttons depending on selected items
         right.addListSelectionListener(new ListSelectionListener() {
        
            private int selectedCount() {
               return right.getSelectedValuesList().size();
            }
        
            @Override
            public void valueChanged(ListSelectionEvent arg0) {
               btnRemove.setEnabled(selectedCount() > 0);
            }
         });

         // Add listener to control btnRemoveAll enabling
         right.getModel().addListDataListener(new ListDataListener() {
        
            private void update() {
               if (right.getModel().getSize() == 0) {
                  btnRemoveAll.setEnabled(false);
               } else {
                  btnRemoveAll.setEnabled(true);
               }
            }
        
            @Override
            public void intervalRemoved(ListDataEvent arg0) {
               update();
            }
        
            @Override
            public void intervalAdded(ListDataEvent arg0) {
               update();
            }
        
            @Override
            public void contentsChanged(ListDataEvent arg0) {
               update();
            }
         });

         // add buttons to the button box
         btnBox.add(Box.createVerticalGlue());
         btnBox.add(Box.createVerticalStrut(BUTTON_HEIGHT));
         btnBox.add(btnAdd);
         btnBox.add(Box.createVerticalStrut(BUTTON_HEIGHT));
         btnBox.add(btnAddAll);
         btnBox.add(Box.createVerticalStrut(BUTTON_HEIGHT));
         btnBox.add(btnRemove);
         btnBox.add(Box.createVerticalStrut(BUTTON_HEIGHT));
         btnBox.add(btnRemoveAll);
         btnBox.add(Box.createVerticalStrut(BUTTON_HEIGHT));
         btnBox.add(Box.createVerticalGlue());
         
         // get button box to the panel
         this.add(btnBox);
      }
   }
   
   private void moveAllOptions(JList<T> from, JList<T> to) {
      /**
       * Move all items from the "from" list to the "to" one
       */
      from.clearSelection();
      to.clearSelection();
      IndigoListModel<T> modelFrom = (IndigoListModel<T>) from
            .getModel();
      IndigoListModel<T> modelTo = (IndigoListModel<T>) to
            .getModel();
      for (int i = 0; i < modelFrom.getSize(); i++) {
         modelTo.addItem(modelFrom.getElementAt(i));
      }
      modelFrom.removeAllItems();
   }

   private void moveSelectedOptions(JList<T> from, JList<T> to) {
      /**
       * Move selected items from the "from" list to the "to" one
       */
      final List<T> list = from.getSelectedValuesList();
      from.clearSelection();
      to.clearSelection();
      for (final T t : list) {
         ((IndigoListModel<T>) from.getModel()).removeItem(t);
         ((IndigoListModel<T>) to.getModel()).addItem(t);
      }
   }
   
   public void saveSettingsTo(IndigoNodeSettings settings) {
      ((IndigoListModel<T>)rightPanel.getList().getModel()).saveSettingsTo(settings);
   }
   
   public void loadSettingsFrom(IndigoNodeSettings settings) {
      ((IndigoListModel<T>)rightPanel.getList().getModel()).removeAllItems();
      ((IndigoListModel<T>)leftPanel.getList().getModel()).removeAllItems();
      ((IndigoListModel<T>)rightPanel.getList().getModel()).loadSettingsFrom(settings);
      ((IndigoListModel<T>)leftPanel.getList().getModel()).loadSettingsFrom(settings);
   }
   
   public IndigoListPanel<T> getLeftPanel() {
      return leftPanel;
   }

   public IndigoListPanel<T> getRightPanel() {
      return rightPanel;
   }
   
   public void uploadItems(T[] items, IndigoListPanel<T> panel) {
      /**
       * upload the items to the panel list
       */
      IndigoListModel<T> model = (IndigoListModel<T>) panel.getList().getModel();
      for (T item: items) {
        model.addItem(item);
      }
   }
   
}
