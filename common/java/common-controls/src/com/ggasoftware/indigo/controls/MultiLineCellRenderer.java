package com.ggasoftware.indigo.controls;

import java.awt.Color;
import java.awt.Component;
import java.awt.Font;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.border.Border;
import javax.swing.border.EmptyBorder;

class MultiLineCellRenderer extends JPanel implements TableCellRenderer {

   public MultiLineCellRenderer(int horizontalAlignment, int verticalAlignment) {
      this.horizontalAlignment = horizontalAlignment;
      this.verticalAlignment = verticalAlignment;
      switch (horizontalAlignment) {
         case SwingConstants.LEFT:
            alignmentX = (float) 0.0;
            break;

         case SwingConstants.CENTER:
            alignmentX = (float) 0.5;
            break;

         case SwingConstants.RIGHT:
            alignmentX = (float) 1.0;
            break;

         default:
            throw new IllegalArgumentException(
                    "Illegal horizontal alignment value");
      }

      setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
      setOpaque(true);
      setBorder(border);

      background = null;
      foreground = null;
   }

   public void setForeground(Color foreground) {
      super.setForeground(foreground);
      Component[] comps = this.getComponents();
      int ncomp = comps.length;
      for (int i = 0; i < ncomp; i++) {
         Component comp = comps[i];
         if (comp instanceof JLabel) {
            comp.setForeground(foreground);
         }
      }
   }

   public void setBackground(Color background) {
      this.background = background;
      super.setBackground(background);
   }

   public void setFont(Font font) {
      this.font = font;
   }

   // Implementation of TableCellRenderer interface
   public Component getTableCellRendererComponent(JTable table, Object value,
           boolean isSelected, boolean hasFocus, int row, int column) {
      removeAll();
      invalidate();

      if (value == null || table == null) {
         // Do nothing if no value
         return this;
      }

      Color cellForeground;
      Color cellBackground;

      // Set the foreground and background colors
      // from the table if they are not set
      cellForeground = (foreground == null ? table.getForeground()
              : foreground);
      cellBackground = (background == null ? table.getBackground()
              : background);

      // Handle selection and focus colors
      if (isSelected == true) {
         cellForeground = table.getSelectionForeground();
         cellBackground = table.getSelectionBackground();
      }

      if (hasFocus == true) {
         setBorder(UIManager.getBorder("Table.focusCellHighlightBorder"));
         if (table.isCellEditable(row, column)) {
            cellForeground = UIManager.getColor("Table.focusCellForeground");
            cellBackground = UIManager.getColor("Table.focusCellBackground");
         }
      } else {
         setBorder(border);
      }

      super.setForeground(cellForeground);
      super.setBackground(cellBackground);

      // Default the font from the table
      if (font == null) {
         font = table.getFont();
      }

      if (verticalAlignment != SwingConstants.TOP) {
         add(Box.createVerticalGlue());
      }

      Object[] values;
      int length;
      if (value instanceof Object[]) {
         // Input is an array - use it
         values = (Object[]) value;
      } else {
         // Not an array - turn it into one
         values = new Object[1];
         values[0] = value;
      }
      length = values.length;

      // Configure each row of the cell using
      // a separate JLabel. If a given row is
      // a JComponent, add it directly..
      for (int i = 0; i < length; i++) {
         Object thisRow = values[i];

         if (thisRow instanceof JComponent) {
            add((JComponent) thisRow);
         } else {
            JLabel l = new JLabel();
            setValue(l, thisRow, i, cellForeground);
            add(l);
         }
      }

      if (verticalAlignment != SwingConstants.BOTTOM) {
         add(Box.createVerticalGlue());
      }
      return this;
   }

   // Configures a label for one line of the cell.
   // This can be overridden by derived classes
   protected void setValue(JLabel l, Object value, int lineNumber,
           Color cellForeground) {
      if (value != null && value instanceof Icon) {
         l.setIcon((Icon) value);
      } else {
         l.setText(value == null ? "" : value.toString());
      }
      l.setHorizontalAlignment(horizontalAlignment);
      l.setAlignmentX(alignmentX);
      l.setOpaque(false);
      l.setForeground(cellForeground);
      l.setFont(font);
   }
   protected int verticalAlignment;
   protected int horizontalAlignment;
   protected float alignmentX;
   // These attributes may be explicitly set
   // They are defaulted to the colors and attributes
   // of the table
   protected Color foreground;
   protected Color background;
   protected Font font;
   protected Border border = new EmptyBorder(1, 2, 1, 2);
}
