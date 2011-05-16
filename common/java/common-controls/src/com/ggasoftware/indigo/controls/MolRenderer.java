package com.ggasoftware.indigo.controls;

import java.awt.*;
import javax.swing.JPanel;
import javax.swing.JTable;
import javax.swing.table.TableCellRenderer;
import java.io.*;
import com.ggasoftware.indigo.*;
import java.awt.image.BufferedImage;
import java.net.URL;
import javax.imageio.stream.MemoryCacheImageInputStream;
import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.UIManager;
import javax.swing.border.EmptyBorder;

public class MolRenderer extends JPanel
        implements TableCellRenderer
{
   static int call_count = 0;
   private BufferedImage image;
   private ImageIcon _exclamation_img;
   Color bg_color;

   public MolRenderer()
   {
      URL url = this.getClass().getResource("images/exclamation.png");
      _exclamation_img = new ImageIcon(url);
   }

   public Component getTableCellRendererComponent(
           JTable table, Object value, boolean isSelected,
           boolean hasFocus, int row, int column)
   {
      if (value == null)
         return null;
      
      RenderableObject mol_image = (RenderableObject) value;

      Indigo indigo = mol_image.getIndigo();
      IndigoRenderer indigo_renderer = mol_image.getIndigoRenderer();

      // To avoid parallel rendering (maybe will be fixed)
      synchronized (indigo)
      {
         indigo.setOption("render-output-format", "png");

         if (mol_image == null)
            return null;

         IndigoObject indigo_obj = mol_image.getRenderableObject();

         int cell_h = table.getRowHeight(row);
         int cell_w = table.getColumnModel().getColumn(column).getWidth();

         if (isSelected)
            bg_color = table.getSelectionBackground();
         else
            bg_color = table.getBackground();
         
         float[] c = bg_color.getComponents(null);
         indigo.setOption("render-background-color", c[0], c[1], c[2]);
         
         if (indigo_obj == null)
         {
            image = new BufferedImage(cell_w, cell_h, BufferedImage.TYPE_INT_RGB);
            Graphics2D gc = image.createGraphics();
            gc.setColor(bg_color);
            gc.fillRect(0, 0, cell_w, cell_h);
            gc.setColor(Color.black);
            gc.drawString("Cannot render", 40, (int) (cell_h / 2));
            gc.drawImage(_exclamation_img.getImage(), 5, 10, null);
            return this;
         }
        
         indigo.setOption("render-image-size", cell_w, cell_h);
         byte[] bytes = null;

         try
         {
            bytes = indigo_renderer.renderToBuffer(indigo_obj);

            //System.out.print("Render: " + call_count + "\n");
            call_count++;

            ByteArrayInputStream bytes_is = new ByteArrayInputStream(bytes, 0, bytes.length);
            image = ImageIO.read(new MemoryCacheImageInputStream(bytes_is));
         }
         catch (IOException ex)
         {
            System.err.println(">>>>" + ex.getMessage());
            ex.printStackTrace();
         }
         if (mol_image.getErrorMessageToRender() != null)
         {
            // Mark molecule somehow
            Graphics2D gc = image.createGraphics();
            gc.drawImage(_exclamation_img.getImage(), 5, 10, null);
         }

         if (hasFocus)
            setBorder(UIManager.getBorder("Table.focusCellHighlightBorder"));
         else
            setBorder(new EmptyBorder(1, 2, 1, 2));
            
         return this;
      }
   }

   @Override
   public void paintComponent(Graphics g)
   {
      setBackground(bg_color);
      if (image != null)
         g.drawImage(image, 0, 0, this);
   }
}
