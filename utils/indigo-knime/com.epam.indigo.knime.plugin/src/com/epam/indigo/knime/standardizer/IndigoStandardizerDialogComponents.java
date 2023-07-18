package com.epam.indigo.knime.standardizer;

import java.awt.Color;
import java.awt.Component;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JScrollPane;
import javax.swing.ListCellRenderer;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

import com.epam.indigo.knime.common.IndigoDialogPanel;
import com.epam.indigo.knime.common.IndigoItemsFilterPanel;
import com.epam.indigo.knime.common.IndigoListModel;
import com.epam.indigo.knime.common.IndigoListPanel;
import com.epam.indigo.knime.common.IndigoNodeSettings;
import com.epam.indigo.knime.common.components.FilteredListModel;
import com.epam.indigo.knime.common.components.FilteredListPanel;
import com.epam.indigo.knime.common.transformer.IndigoTransformerSettings;
import com.epam.indigo.knime.common.transformer.IndigoTransformerNodeDialog.DialogComponents;
import com.epam.indigo.knime.standardizer.IndigoStandardizerUtils.Option;

public class IndigoStandardizerDialogComponents implements DialogComponents {

   final IndigoItemsFilterPanel<Option> optionsFilter = new IndigoItemsFilterPanel<Option>(
         new FilteredListPanel<Option>(new FListModel(), new OptionCellRenderer()), 
         new RightPanel());
   
   @Override
   public void loadDialogComponents(IndigoDialogPanel dialogPanel,
         IndigoTransformerSettings settings) {
      
      Map<String, Option> map = IndigoStandardizerUtils.optionsMap;
      Option[] options =  map.values().toArray(new Option[map.size()]);
      
      optionsFilter.uploadItems(options, optionsFilter.getLeftPanel());
      
      dialogPanel.addItemsPanel("Options");
      dialogPanel.addItem(optionsFilter);
      
   }
   
   @Override
   public void saveToSettings(IndigoTransformerSettings settings) {
      optionsFilter.saveSettingsTo(settings);
   }
   
   @Override
   public void loadFromSettings(IndigoTransformerSettings settings) {
      optionsFilter.loadSettingsFrom(settings);
   }
   
   @SuppressWarnings("serial")
   class RightPanel extends IndigoListPanel<Option> {
      
      public RightPanel() {
         
         super(new SwappingList(new SwappingModelList()), new OptionCellRenderer());

         final SwappingList toApply = (SwappingList) this.getList();
         
         JScrollPane pcScroller = new JScrollPane(toApply);
         pcScroller.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

         GridBagLayout gbl = new GridBagLayout();
         gbl.columnWidths = new int[]{0, 0};
         gbl.rowHeights = new int[]{0, 0, 0};
         gbl.columnWeights = new double[]{1.0, Double.MIN_VALUE};
         gbl.rowWeights = new double[]{0.0, 1.0, Double.MIN_VALUE};
         this.setLayout(gbl);

         final JButton btnUp = new JButton(" Up ");
         btnUp.setEnabled(false);
         GridBagConstraints gbcBtnUp = new GridBagConstraints();
         gbcBtnUp.gridx = 2;
         gbcBtnUp.gridy = 0;
         this.add(btnUp, gbcBtnUp);

         final JButton btnDown = new JButton("Down");
         btnDown.setEnabled(false);
         GridBagConstraints gbcBtnDown = new GridBagConstraints();
         gbcBtnDown.gridx = 1;
         gbcBtnDown.gridy = 0;
         this.add(btnDown, gbcBtnDown);

         GridBagConstraints gbcList = new GridBagConstraints();
         gbcList.fill = GridBagConstraints.BOTH;
         gbcList.gridwidth = 3;
         gbcList.gridx = 0;
         gbcList.gridy = 1;
         this.add(pcScroller, gbcList);
       
         // Add a behavioral listener to Up button
         btnUp.addActionListener(new ActionListener() {
    
            @Override
            public void actionPerformed(ActionEvent arg0) {
               Option option = toApply.getSelectedValue();
               toApply.moveUp(option);
            }
         });

         // Add a behavioral listener to Down button
         btnDown.addActionListener(new ActionListener() {
          
            @Override
            public void actionPerformed(ActionEvent arg0) {
               Option option = toApply.getSelectedValue();
               toApply.moveDown(option);
            }
         });
         
         // Add listener which disables buttons depending on selected items
         toApply.addListSelectionListener(new ListSelectionListener() {
        
            private int selectedCount() {
               return toApply.getSelectedValuesList().size();
            }
        
            @Override
            public void valueChanged(ListSelectionEvent arg0) {
               btnDown.setEnabled(selectedCount() == 1);
               btnUp.setEnabled(selectedCount() == 1);
            }
         });
         
      }
   }
   
   @SuppressWarnings("serial")
   class FListModel extends FilteredListModel<Option> {
      /**
       * List model class
       */
      
      @Override
      public void saveSettingsTo(IndigoNodeSettings settings) {
      }
      
      @Override
      public void loadSettingsFrom(IndigoNodeSettings settings) {
         
         Map<String, Option> map = IndigoStandardizerUtils.optionsMap;
         
         Set<String> picked = new HashSet<String>(Arrays.asList(
               ((IndigoStandardizerNodeSettings)settings).pickedOptions.getStringArrayValue()));
         Set<String> all = map.keySet();
         // add all the unpicked options to the left list
         for (String option: all) {
            if (!picked.contains(option) && !containItem(map.get(option)))
               addItem(IndigoStandardizerUtils.optionsMap.get(option));
         }
      }
   }
   
   @SuppressWarnings("serial")
   class SwappingList extends JList<Option> {
      /**
       * Swapping list view class. Provide ability to swap own neighbor elements
       */
      
      public SwappingList(SwappingModelList model) {
         super(model);
      }
      
      // Move item one step up if possible
      public void moveUp(Option item) {
         SwappingModelList model = (SwappingModelList)getModel();
         ArrayList<Option> items = model.getItems();
         if (items.contains(item)) {
            int idx = -1;
            for (int i = 0; i < items.size(); i++) {
               if (items.get(i).equals(item)) {
                  idx = i;
                  break;
               }
            }
            if (idx > 0 && items.size() > 1) {
               model.swap(idx - 1, idx);
               this.clearSelection();
               this.setSelectedIndex(idx - 1);
            }
         }
      }

      // Move item one step down if possible
      public void moveDown(Option item) {
         SwappingModelList model = (SwappingModelList)getModel();
         ArrayList<Option> items = model.getItems();
         if (items.contains(item)) {
            int idx = -1;
            for (int i = 0; i < items.size(); i++) {
               if (items.get(i).equals(item)) {
                  idx = i;
                  break;
               }
            }
            if (idx < items.size() - 1 && items.size() > 1) {
               model.swap(idx, idx + 1);
               this.clearSelection();
               this.setSelectedIndex(idx + 1);
            }
         }
      }
   }

   @SuppressWarnings("serial")
   class SwappingModelList extends IndigoListModel<Option> {
      /**
       * List model class 
       */

      ArrayList<Option> items;
      
      public SwappingModelList() {
         super();
         items = new ArrayList<Option>();
      }
      
      public ArrayList<Option> getItems() {
         return items;
      }
      
      // Swap i and j elements
      public void swap(int i, int j) throws IndexOutOfBoundsException {
         Collections.swap(items, i, j);
         fireContentsChanged(this, 0, getSize());
      }
      
      @Override
      public Option getElementAt(int index) {
         return (index < items.size()) ? items.get(index) : null;
      }

      @Override
      public int getSize() {
         return items.size();
      }

      @Override
      public void addItem(Option item) {
         items.add(item);
         fireContentsChanged(this, 0, getSize());
      }

      @Override
      public void removeItem(Option item) {
         items.remove(item);
         fireContentsChanged(this, 0, getSize());
      }

      @Override
      public void removeAllItems() {
         items.removeAll(items);
         fireContentsChanged(this, 0, getSize());
      }

      @Override
      public void saveSettingsTo(IndigoNodeSettings settings) {
          /**
          * Saves settings from the dialog to the settings object
          */
         String[] picked = new String[getSize()];
         for (int i = 0; i < picked.length; i++) {
            picked[i] = (getElementAt(i)).getName();
         }
         ((IndigoStandardizerNodeSettings)settings)
            .pickedOptions.setStringArrayValue(picked);
      }

      @Override
      public void loadSettingsFrom(IndigoNodeSettings settings) {
         String[] picked = ((IndigoStandardizerNodeSettings)settings)
                                       .pickedOptions.getStringArrayValue();
         for (int i = 0; i < picked.length; i++) {
            addItem(IndigoStandardizerUtils.optionsMap.get(picked[i]));
         }
      }
   }
   
   @SuppressWarnings("serial")
   class OptionCellRenderer extends JLabel implements ListCellRenderer<Option> {
      /**
       * Class for rendering JList option item
       */

      public OptionCellRenderer() {
         setOpaque(true);
      }

      @Override
      public Component getListCellRendererComponent(
            JList<? extends Option> list, Option value, int index,
            boolean isSelected, boolean cellHasFocus) {
         Option option = (Option) value;
         // define items view and tooltips
         setText(option.getName());
         setToolTipText(option.getDescription() + ". Group: "
               + option.getGroup() + ".");
         if (isSelected) {
            setBackground(Color.getHSBColor(0.56f, 0.9f, 0.8f)); 
            setForeground(Color.white);
         } else {
            setBackground(Color.white);
            setForeground(Color.black);
         }
         return this;
      }
   }
}
   
