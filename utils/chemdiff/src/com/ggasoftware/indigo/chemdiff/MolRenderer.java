
package com.ggasoftware.indigo.chemdiff;

import java.awt.*;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JPanel;
import javax.swing.JTable;
import javax.swing.table.TableCellRenderer;
import java.io.*;
import com.ggasoftware.indigo.*;
import java.awt.image.BufferedImage;
import java.awt.image.ImageObserver;
import javax.imageio.stream.MemoryCacheImageInputStream;
import javax.imageio.ImageIO;

public class MolRenderer extends JPanel
                     implements TableCellRenderer
{
  private Indigo indigo;
  private IndigoRenderer indigo_renderer;
  private IndigoObject indigo_obj;
  private BufferedImage image;
  private ImageIO image_io;
  private boolean is_reactions_mode;
  private MolViewTable mol_table;

  int cell_w;
  int cell_h;

  public MolRenderer( Indigo cur_indigo, IndigoRenderer cur_indigo_renderer,
                      MolViewTable cur_mol_table,
                      int new_cell_w, int new_cell_h, boolean is_reactions )
  {
     mol_table = cur_mol_table;
     indigo = cur_indigo;
     indigo_renderer = cur_indigo_renderer;
     cell_w = new_cell_w;
     cell_h = new_cell_h;
     indigo.setOption("render-output-format", "png");
     indigo.setOption("render-background-color", "1,1,1");
     indigo.setOption("render-coloring", "1");
     indigo.setOption("render-comment-font-size", "14");
     is_reactions_mode = is_reactions;
  }

  public class ImageObs implements ImageObserver
  {
     public boolean imageUpdate(Image img, int infoflags, int x, int y, int width, int height) {
        return true;
     }
  }

   @SuppressWarnings("static-access")
  public Component getTableCellRendererComponent(
              JTable table, Object value, boolean isSelected,
              boolean hasFocus, int row, int column)
  {
     if (mol_table.getSdfLoader().isActive())
        return this;

     MolCell mol_image = (MolCell)value;

     if (mol_image.object == null)
     {
        mol_image.image = new BufferedImage(cell_w, cell_h, BufferedImage.TYPE_INT_RGB);
        Graphics2D gc = mol_image.image.createGraphics();
        gc.setColor(Color.white);
        gc.fillRect(0, 0, cell_w, cell_h);
        gc.setColor(Color.black);
        gc.drawString("Cannot render", 10, (int)(cell_h/2));
     }

     if (mol_image.image != null)
     {
        image = mol_image.image;
        return this;
     }

     indigo_obj = mol_image.object.clone();

     //indigo.setOption("render-comment", indigo_obj.name());

     String size_str = "" + (cell_w) + ',' + (cell_h);
     indigo.setOption("render-image-size", size_str);
     byte[] bytes = null;

     Boolean valid = false;
     try
     {
        indigo_obj.canonicalSmiles();
        valid = true;
     }
     catch ( Exception ex )
     {
     }
     
     bytes = indigo_renderer.renderToBuffer(indigo_obj);

     ByteArrayInputStream bytes_is = new ByteArrayInputStream(bytes, 0, bytes.length);
     try {
        image = image_io.read(new MemoryCacheImageInputStream(bytes_is));
     } catch (IOException ex) {
        System.err.println(">>>>" + ex.getMessage() );
        ex.printStackTrace();
     }

     if (!valid)
     {
        // Mark molecule somehow
        Graphics2D gc = image.createGraphics();
        gc.setColor(Color.red);
        gc.drawString("No canonical SMILES", 10, 10);
     }
     mol_image.image = image;

     return this;
  }

  public void paintComponent(Graphics g)
  {
     if (image != null)
        g.drawImage(image, 0, 0, new ImageObs());
  }
}
