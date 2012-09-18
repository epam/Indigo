/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import javax.swing.SwingWorker;

public class IndigoObjectsFileLoader extends 
        SwingWorker<List<IndigoObjectWrapper>, Void>
{
   private Indigo indigo;
   private File file;
   
   private boolean useProxyObjects = true;
   
   public IndigoObjectsFileLoader (Indigo indigo, File file)
   {
      this.indigo = indigo;
      this.file = file;
   }
   
   public void setUseProxyObject (boolean useProxyObjects)
   {
       this.useProxyObjects = useProxyObjects;
   }
   
   @Override
   protected List<IndigoObjectWrapper> doInBackground () throws Exception
   {
      IndigoObject iterator_object = 
              CommonUtils.getIterator(indigo, file.getPath());
      
      int count = iterator_object.count();
      ArrayList<IndigoObjectWrapper> objects = new ArrayList<IndigoObjectWrapper>();
      
      for (IndigoObject item: iterator_object)
      {
         if (isCancelled())
            return null;
         
         int index = item.index();
         IndigoObjectWrapper obj;
         if (useProxyObjects)
            obj = new IndigoIteratorItem(iterator_object, index);
         else
            obj = new PureIndigoObject(item.clone());
             
         objects.add(obj);

         setProgress(100 * index / count);
      }
      return objects;
   }
}
