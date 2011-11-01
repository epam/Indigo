package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.*;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.awt.image.ImageObserver;
import java.io.ByteArrayInputStream;
import javax.imageio.ImageIO;
import javax.imageio.stream.MemoryCacheImageInputStream;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

public class IndigoObjectViewPanel extends JPanel
{
   private BufferedImage image;
   private ImageIO image_io;
   private Indigo indigo;
   private IndigoRenderer indigo_renderer;
   private IndigoObject chem_obj;
   private String error_string = null;
   int image_w;
   int image_h;

   public IndigoObjectViewPanel ()
   {
      setBackground(new java.awt.Color(255, 255, 255));
      setImageSize(getWidth(), getHeight());
   }

   public void renderImage ()
   {
      try
      {
         byte[] bytes;
         final Indigo indigo_sync = indigo;
         synchronized (indigo_sync)
         {
            indigo.setOption("render-output-format", "png");
            indigo.setOption("render-comment-font-size", "14");
            indigo.setOption("render-background-color", "1,1,1");
            indigo.setOption("render-coloring", "1");
            indigo.setOption("render-image-size", image_w, image_h);

            try
            {
               bytes = indigo_renderer.renderToBuffer(chem_obj);
            }
            catch (Exception ex)
            {
               image = null;
               return;
            }
         }

         ByteArrayInputStream bytes_is;
         bytes_is = new ByteArrayInputStream(bytes, 0, bytes.length);

         image = image_io.read(new MemoryCacheImageInputStream(bytes_is));

         updateUI();
      }
      catch (Exception ex)
      {
         JOptionPane msg_box = new JOptionPane();
         msg_box.showMessageDialog(this, ex.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
         setErrorString(ex.getMessage());
      }
   }

   public void setIndigoObject (IndigoObject chem_obj, IndigoRenderer indigo_renderer)
   {
      error_string = null;

      if (chem_obj == null)
      {
         image = null;
         this.chem_obj = null;
         return;
      }

      this.indigo = chem_obj.getIndigo();
      this.indigo_renderer = indigo_renderer;
      this.chem_obj = chem_obj;

      renderImage();
   }

   public void update ()
   {
      revalidate();
      repaint();
   }

   public void setImageSize (int width, int height)
   {
      image_w = width;
      image_h = height;
   }

   public class ImageObs implements ImageObserver
   {
      @SuppressWarnings("static-access")
      public boolean imageUpdate (Image img, int infoflags, int x, int y, int width, int height)
      {
         return true;
      }
   }

   public void setErrorString (String str)
   {
      error_string = new String(str);
      update();
   }

   public String getErrorString ()
   {
      return error_string;
   }

   @Override
   public void paintComponent (Graphics g)
   {
      g.setColor(Color.white);
      g.fillRect(0, 0, getWidth(), getHeight());

      if (image != null)
      {
         g.setColor(Color.white);
         if ((image_w != getWidth()) || (image_h != getHeight()))
         {
            setImageSize(getWidth(), getHeight());
            renderImage();
         }

         double new_im_h = image.getHeight();
         double new_im_w = image.getWidth();

         g.drawImage(image, getWidth() / 2 - (int) new_im_w / 2,
                 getHeight() / 2 - (int) new_im_h / 2,
                 (int) new_im_w, (int) new_im_h, new ImageObs());
      }

      if (error_string != null)
      {
         g.setColor(Color.black);
         g.drawString(error_string, 10, (int)getHeight() / 2);
      }
   }
}
