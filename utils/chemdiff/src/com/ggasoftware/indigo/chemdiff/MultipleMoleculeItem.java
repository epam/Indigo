package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.IndigoRenderer;
import com.ggasoftware.indigo.controls.IndigoCheckedException;
import com.ggasoftware.indigo.controls.RenderableObject;
import java.util.ArrayList;

public class MultipleMoleculeItem extends RenderableObjectWithId
{
   private int _group_count;
   private ArrayList<ArrayList<MoleculeItem>> _groups;
   private CanonicalCodeGenerator _canonical_generator;
   private String _canonical_code;

   public MultipleMoleculeItem(int group_count, CanonicalCodeGenerator canonical_generator)
   {
      _group_count = group_count;
      _groups = new ArrayList<ArrayList<MoleculeItem>>();
      for (int i = 0; i < _group_count; i++)
         _groups.add(new ArrayList<MoleculeItem>());
      _canonical_generator = canonical_generator;
   }

   public MultipleMoleculeItem (MoleculeItem mol, CanonicalCodeGenerator canonical_generator)
   {
      this(1, canonical_generator);
      _getGroup(0).add(mol);
   }

   public ArrayList<MoleculeItem> getGroup(int group_index)
   {
      return _getGroup(group_index);
   }

   private ArrayList<MoleculeItem> _getGroup(int group_index)
   {
      return _groups.get(group_index);
   }

   public int getGroupCount ()
   {
      return _group_count;
   }

   @Override
   public String getId()
   {
      if (isSingleMolecule())
         return getId(0);
      throw new RuntimeException("getId() has no implementation");
   }
           
   @Override
   public String getId(int group_index)
   {
      // Merge ids from corresponding group
      StringBuilder sb = new StringBuilder();
      for (MoleculeItem item: getGroup(group_index))
      {
         if (sb.length() > 0)
            sb.append("\n");
         sb.append(item.getId());
      }
      return sb.toString();
   }

   public boolean isSingleMolecule ()
   {
      return getGroupCount() == 1 && getGroup(0).size() == 1;
   }
   
   @Override
   public IndigoObject getRenderableObject()
   {
      // TODO: set name too...
      
      RenderableObject rend_obj = _groups.get(0).get(0);
      if (isSingleMolecule())
      {
         IndigoObject obj = rend_obj.getRenderableObject();
         if (obj == null)
            setErrorMessageToRender(rend_obj.getErrorMessageToRender());
         return obj;
      }
         
      // For rendering pick any object from this group
      // and prepare by the canonical code generator
      MoleculeItem m = _groups.get(0).get(0);
      IndigoObject obj_prepared;
      try
      {
         obj_prepared = _canonical_generator.createPreparedObject(m);
      }
      catch (IndigoCheckedException ex)
      {
         setErrorMessageToRender(ex.getMessage());
         return null;
      }
      obj_prepared.markEitherCisTrans();

      // Clear properties
      obj_prepared.clearProperties();
      if (getGroupCount() == 1)
         obj_prepared.setProperty("Id", getId(0));
      else
      {
         obj_prepared.setProperty("Id1", getId(0));
         obj_prepared.setProperty("Id2", getId(1));
      }
      return obj_prepared;
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
      IndigoObject obj = getRenderableObject();
      if (obj == null)
         throw new IndigoCheckedException(getErrorMessageToRender());
      return obj;
   }
   
   public String getCanonicalCode ()
   {
      return _canonical_code;
   }
   public void setCanonicalCode (String code)
   {
      _canonical_code = code;
   }
}
