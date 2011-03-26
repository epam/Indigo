package com.ggasoftware.indigo.controls;

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
  static int call_count = 0;

  private Indigo indigo;
  private IndigoRenderer indigo_renderer;
  private IndigoObject indigo_obj;
  private BufferedImage image;
  private boolean is_reactions_mode;

  int cell_w;
  int cell_h;

  public MolRenderer( Indigo cur_indigo, IndigoRenderer cur_indigo_renderer,
                      int new_cell_w, int new_cell_h, boolean is_reactions )
  {
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

  public synchronized Component getTableCellRendererComponent(
              JTable table, Object value, boolean isSelected,
              boolean hasFocus, int row, int column)
  {
     RenderableObject mol_image = (RenderableObject)value;
     synchronized (indigo) {

        if (mol_image == null)
           return null;

        try {
           indigo_obj = mol_image.getObject();
        } catch (Exception ex)  {
           indigo_obj = null;
        }

        if (indigo_obj == null)
        {
           image = new BufferedImage(cell_w, cell_h, BufferedImage.TYPE_INT_RGB);
           Graphics2D gc = image.createGraphics();
           gc.setColor(Color.white);
           gc.fillRect(0, 0, cell_w, cell_h);
           gc.setColor(Color.black);
           gc.drawString("Cannot render", 10, (int)(cell_h/2));

           return this;
        }

        indigo.setOption("render-image-size", cell_w, cell_h);
        byte[] bytes = null;

        Boolean valid = false;
        try
        {
           //indigo_obj.canonicalSmiles();
           valid = true;
        }
        catch ( Exception ex )
        {
        }

        bytes = indigo_renderer.renderToBuffer(indigo_obj.clone());

        System.out.print("Render: " + call_count + "\n");
        call_count++;

        ByteArrayInputStream bytes_is = new ByteArrayInputStream(bytes, 0, bytes.length);
        try {
           synchronized (ImageIO.class) {
              image = ImageIO.read(new MemoryCacheImageInputStream(bytes_is));
           }
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

        return this;
     }
  }

  public void paintComponent(Graphics g)
  {
     if (image != null)
        g.drawImage(image, 0, 0, new ImageObs());
  }
}
