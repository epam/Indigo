package com.epam.indigo.controls;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.IndigoRenderer;

public class MoleculeItem extends RenderableObjectWithId implements IndigoObjectWrapper
{
   private IndigoObjectWrapper _object;
   private String _id;

   public MoleculeItem (IndigoObjectWrapper item, String id)
   {
      _object = item;
      _id = id;
   }

   public void setId (String id)
   {
      _id = id;
   }

   @Override
   public String getId ()
   {
      return _id;
   }

   @Override
   public String getId (int index)
   {
      if (index > 0)
         throw new RuntimeException("index > 0");
      return getId();
   }

   @Override
   public IndigoObject getRenderableObject ()
   {
      final Indigo indigo = getIndigo();
      synchronized (getIndigo())
      {
         // Set ignore errors for rendering
         indigo.setOption("ignore-stereochemistry-errors", "true");
         try
         {
            return _object.getObjectCopy();
         }
         catch (IndigoCheckedException ex)
         {
            setErrorMessageToRender(ex.getMessage());
            return null;
         }
      }
   }

   @Override
   public Indigo getIndigo ()
   {
      return Global.indigo;
   }

   @Override
   public IndigoRenderer getIndigoRenderer ()
   {
      return Global.indigo_renderer;
   }

   public IndigoObject getObjectCopy () throws IndigoCheckedException
   {
      return _object.getObjectCopy();
   }
}
