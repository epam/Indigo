/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems, Inc.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 ***************************************************************************/

package com.epam.indigo.knime.cell;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;

import javax.imageio.ImageIO;

import com.epam.indigo.*;
import com.epam.indigo.knime.plugin.IndigoPlugin;

import org.knime.chem.types.CMLValue;
import org.knime.chem.types.MolValue;
import org.knime.chem.types.RxnValue;
import org.knime.chem.types.SdfValue;
import org.knime.chem.types.SmartsValue;
import org.knime.chem.types.SmilesValue;
import org.knime.core.data.AdapterCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataType;
import org.knime.core.data.renderer.*;
import org.knime.core.node.NodeLogger;

@SuppressWarnings("serial")
public class IndigoDataValueRenderer extends AbstractPainterDataValueRenderer
{
   private static final NodeLogger LOGGER = NodeLogger
         .getLogger(IndigoDataValueRenderer.class);
   
   /**
    * 
    * Indigo data value renderer factory.
    *
    */
   public static final class Factory extends AbstractDataValueRendererFactory {

      private static final String DESCRIPTION = "Indigo renderer";
      
      /**
       * {@inheritDoc}
       */
      @Override
      public DataValueRenderer createRenderer(final DataColumnSpec colSpec) {
          return new IndigoDataValueRenderer();
      }

      @Override
      public String getDescription() {
         return DESCRIPTION;
      }

   }

   private static final Font NO_2D_FONT = new Font(Font.SANS_SERIF, Font.ITALIC, 12);

   IndigoObject object = null;
   String errorMessage = null;

   private static IndigoRenderer renderer = null;

   private IndigoObject convertIntoIndigoObject(Object obj) {
      
      Indigo indigo = IndigoPlugin.getIndigo();
      IndigoObject io = null;
      
      if (obj instanceof MolValue)
         io = indigo.loadQueryMolecule(((MolValue) obj).getMolValue());
      else if (obj instanceof SdfValue)
         io = indigo.loadQueryMolecule(((SdfValue) obj).getSdfValue());
      else if (obj instanceof SmilesValue) {
         String smiles = ((SmilesValue) obj).getSmilesValue();
         io = !smiles.matches("^[^>]*>[^>]*>[^>]*$") ? indigo.loadMolecule(smiles) : indigo.loadReaction(smiles);
      } else if (obj instanceof SmartsValue) {
         String smarts = ((SmartsValue) obj).getSmartsValue();
         io = !smarts.matches("^[^>]*>[^>]*>[^>]*$") ? indigo.loadSmarts(smarts) : indigo.loadQueryReaction(smarts);
      }
      else if (obj instanceof RxnValue)
         io = indigo.loadQueryReaction((((RxnValue) obj).getRxnValue()));
      else if (obj instanceof CMLValue)
         io = indigo.loadMolecule((((CMLValue) obj).getCMLValue()));
      
      return io;
      
   }
   
   /**
    * Instantiates new renderer.
    */
   public IndigoDataValueRenderer()
   {
   }

   /**
    * Sets the string object for the cell being rendered.
    * 
    * @param value
    *           the string value for this cell; if value is <code>null</code> it
    *           sets the text value to an empty string
    * @see javax.swing.JLabel#setText
    * 
    */
   @Override
   protected void setValue (final Object value)
   {
      try {
         IndigoPlugin.lock();
         
         // check if the object is an adapter cell
         if (value instanceof AdapterCell) {
            AdapterCell cell = (AdapterCell) value;
            
            // check if the adapter contains an indigo representation
            if (cell.isAdaptable(IndigoMolValue.class))
               object = cell.getAdapter(IndigoMolValue.class).getIndigoObject();
            else if (cell.isAdaptable(IndigoReactionValue.class))
               object = cell.getAdapter(IndigoReactionValue.class).getIndigoObject();
            else if (cell.isAdaptable(IndigoQueryMolValue.class))
               object = cell.getAdapter(IndigoQueryMolValue.class).getIndigoObject();
            else if (cell.isAdaptable(IndigoQueryReactionValue.class))
               object = cell.getAdapter(IndigoQueryReactionValue.class).getIndigoObject();
            
            // if there is no an indigo representation, then check for standard representations
            // as it still can be adapter cell
            object = convertIntoIndigoObject(value);
            
         }
         else if (value == DataType.getMissingCell()) {
            object = null;
            errorMessage = "Missing cell";
         } else {
            
            // this is the case when cell is not an adapter, but normal
            object = convertIntoIndigoObject(value);
            
         }
      } catch (IndigoException e) {
         object = null;
         errorMessage = e.getMessage();
      } finally {
         IndigoPlugin.unlock();
      }
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected void paintComponent (final Graphics g)
   {
      super.paintComponent(g);
      if (object == null)
      {
         g.setFont(NO_2D_FONT);
         g.drawString(errorMessage == null ? "empty" : errorMessage, 2, 14);
         return;
      }
      
      if (!IndigoPlugin.getDefault().isRenderingEnabled())
      {
         String str = "";
         
         try
         {
            str = object.smiles();
         }
         catch (Exception e)
         {
         }
         
         g.setFont(NO_2D_FONT);
         g.drawString(str, 2, 14);
         g.drawString("rendering disabled by user preference", 2, 34);
         return;
      }
      
      Dimension d = getSize();
      byte[] buf;

      try
      {
         IndigoPlugin.lock();
         
         Indigo indigo = IndigoPlugin.getIndigo();

         if (renderer == null)
            renderer = new IndigoRenderer(indigo);

         indigo.setOption("render-image-size", d.width, d.height);
         indigo.setOption("render-output-format", "png");
         indigo.setOption("render-bond-length", IndigoPlugin.getDefault().bondLength());
         indigo.setOption("render-implicit-hydrogens-visible", IndigoPlugin.getDefault().showImplicitHydrogens());
         indigo.setOption("render-valences-visible", IndigoPlugin.getDefault().isValencesVisible());
         indigo.setOption("render-coloring", IndigoPlugin.getDefault().coloring());
         indigo.setOption("render-label-mode", IndigoPlugin.getDefault().labelMode());
         indigo.setOption("render-superatom-mode", IndigoPlugin.getDefault().superatomMode());
         indigo.setOption("render-background-color", IndigoPlugin.getDefault().backgroundColor());
         indigo.setOption("render-base-color", IndigoPlugin.getDefault().baseColor());
         indigo.setOption("render-highlight-thickness-enabled", true);
         indigo.setOption("render-highlight-color", 0.7f, 0, 0);
         buf = renderer.renderToBuffer(object);
      }
      catch (IndigoException e)
      {
         g.setFont(NO_2D_FONT);
         g.drawString(e.getMessage(), 2, 14);
         return;
      }
      finally
      {
         IndigoPlugin.unlock();
      }

      try
      {
         BufferedImage img = ImageIO.read(new ByteArrayInputStream(buf));
         g.drawImage(img, 0, 0, null);
      }
      catch (Exception e)
      {
         LOGGER.debug(e.getMessage(), e);
      }

   }

   /**
    * {@inheritDoc}
    */
   @Override
   public String getDescription ()
   {
      return "Indigo Renderer";
   }

   @Override
   public Dimension getPreferredSize ()
   {
      return new Dimension(IndigoPlugin.getDefault().molImageWidth(),
            IndigoPlugin.getDefault().molImageHeight());
   }
}
