package com.epam.indigo.knime.common;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Container;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.util.ArrayList;

import javax.swing.BorderFactory;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.Spring;
import javax.swing.SpringLayout;
import javax.swing.UIManager;
import javax.swing.border.TitledBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.knime.core.node.util.ColumnSelectionComboxBox;

public class IndigoDialogPanel {

   public static final int DEFAULT_PAD = 5;
   public static final int COLUMNS_NUMBER = 2;

   private JPanel resultPanel;
   private JPanel currentPanel;

   private final ArrayList<JPanel> settingsPanels = new ArrayList<JPanel>();

   public IndigoDialogPanel() {
   }

   public JPanel getPanel() {
      if(resultPanel == null) {
         JPanel allSettingsPanel = new JPanel(new GridBagLayout());
         GridBagConstraints c = new GridBagConstraints();
         c.anchor = GridBagConstraints.NORTHWEST;
         c.fill = GridBagConstraints.HORIZONTAL;
         c.gridheight = 1;
         c.gridwidth = 1;
         c.insets = new Insets(0, 0, 0, 0);
         c.ipadx = 0;
         c.ipady = 0;
         c.gridx = 0;
         c.gridy = GridBagConstraints.RELATIVE;
         /*
          * Add all sub panels
          */
         for (JPanel jPanel : settingsPanels) {
            makeCompactGrid(jPanel, COLUMNS_NUMBER);
            allSettingsPanel.add(jPanel, c);
         }
         resultPanel = new JPanel(new BorderLayout());
         resultPanel.add(allSettingsPanel, BorderLayout.PAGE_START);
      }
      return resultPanel;
   }

   public void addItemsPanel(final String name) {
      currentPanel = new JPanel(new SpringLayout());
      settingsPanels.add(currentPanel);

      if(name != null) {
         TitledBorder border = BorderFactory.createTitledBorder(name);
         setDefaultBorderFont(border);
         currentPanel.setBorder(border);
      }
   }

   public void addItem(final String itemLabel, final JComponent itemComponent) {
      JLabel label = new JLabel(itemLabel);
      addItem(label, itemComponent);
   }

   public void addItem(final JComponent leftComponent) {
      addItem(leftComponent, new JPanel());
   }

   public void addItem(final JComponent leftComponent, final JComponent rightComponent) {
      addItem(leftComponent, rightComponent, BorderLayout.WEST, BorderLayout.EAST);
   }
   
   public void addItem(final JComponent leftComponent, final JComponent rightComponent, 
                                       String leftComponetAlignment, String rightComponetAlignment) {
      if(currentPanel == null) {
         throw new RuntimeException("internal error: could not add to empty panel: call addItemsPanel() first");
      }
      if(leftComponent == null || rightComponent == null) {
        throw new RuntimeException("internal error: can not add null components");
    }
      /*
       * Refresh common panel every time
       */
      resultPanel = null;

      /*
       * Add left component
       */
      JPanel leftPanel = new JPanel(new BorderLayout());
      leftPanel.add(leftComponent, leftComponetAlignment);
      setDefaultComponentFont(leftComponent);
      currentPanel.add(leftPanel);

      /*
       * Add right component
       */
      JPanel rightPanel = new JPanel(new BorderLayout());
      rightPanel.add(rightComponent, rightComponetAlignment);
      setDefaultComponentFont(rightComponent);
      currentPanel.add(rightPanel);

   }

   /*
    * Used by makeCompactGrid.
    */
   protected static SpringLayout.Constraints getConstraintsForCell(final int row, final int col, final Container parent, final int cols) {
      SpringLayout layout = (SpringLayout) parent.getLayout();
      Component c = parent.getComponent(row * cols + col);
      return layout.getConstraints(c);
   }

   /**
    * Aligns the first <code>rows</code> * <code>cols</code> components of
    * <code>parent</code> in a grid. Each component in a column is as wide as
    * the maximum preferred width of the components in that column; height is
    * similarly determined for each row. The parent is made just big enough to
    * fit them all.
    *
    */
   protected static void makeCompactGrid(final Container parent, final int cols) {
      int rows = parent.getComponentCount() / cols;
      SpringLayout layout;
      try {
         layout = (SpringLayout) parent.getLayout();
      } catch (ClassCastException exc) {
         System.err.println("The first argument to makeCompactGrid must use SpringLayout.");
         return;
      }

      // Align all cells in each column and make them the same width.
      Spring x = Spring.constant(DEFAULT_PAD);
      for (int c = 0; c < cols; c++) {
         Spring width = Spring.constant(0);
         for (int r = 0; r < rows; r++) {
            width = Spring.max(width, getConstraintsForCell(r, c, parent, cols).getWidth());
         }
         for (int r = 0; r < rows; r++) {
            SpringLayout.Constraints constraints = getConstraintsForCell(r, c, parent, cols);
            constraints.setX(x);
            constraints.setWidth(width);
         }
         x = Spring.sum(x, Spring.sum(width, Spring.constant(DEFAULT_PAD)));
      }

      // Align all cells in each row and make them the same height.
      Spring y = Spring.constant(DEFAULT_PAD);
      for (int r = 0; r < rows; r++) {
         Spring height = Spring.constant(0);
         for (int c = 0; c < cols; c++) {
            height = Spring.max(height, getConstraintsForCell(r, c, parent, cols).getHeight());
         }
         for (int c = 0; c < cols; c++) {
            SpringLayout.Constraints constraints = getConstraintsForCell(r, c, parent, cols);
            constraints.setY(y);
            constraints.setHeight(height);
         }
         y = Spring.sum(y, Spring.sum(height, Spring.constant(DEFAULT_PAD)));
      }

      // Set the parent's size.
      SpringLayout.Constraints pCons = layout.getConstraints(parent);
      pCons.setConstraint(SpringLayout.SOUTH, y);
      pCons.setConstraint(SpringLayout.EAST, x);
   }

   public static void addColumnChangeListener(final JCheckBox appendColumn, final ColumnSelectionComboxBox colName, final JTextField newColName, final String suffix) {
      appendColumn.addChangeListener(new ChangeListener() {
         public void stateChanged(final ChangeEvent e) {
            if (appendColumn.isSelected()) {
               newColName.setEnabled(true);
               if ("".equals(newColName.getText())) {
                  newColName.setText(colName.getSelectedColumn() + suffix);
               }
            } else {
               newColName.setEnabled(false);
            }
         }
      });
      newColName.setEnabled(appendColumn.isSelected());
   }

   public static void setDefaultComponentFont(final JComponent button) {
      /*
       * Not used at the moment
       */
   }
   private static void setDefaultBorderFont(final TitledBorder border) {
      Font titleFont = border.getTitleFont();
      if (titleFont == null) {
          // see http://bugs.sun.com/view_bug.do?bug_id=7022041
          titleFont = UIManager.getDefaults().getFont("TitledBorder.font");
      }
      if (titleFont != null) {
          border.setTitleFont(new Font(titleFont.getFontName(), Font.BOLD, titleFont.getSize()));
      }
   }

}
