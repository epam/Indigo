package com.ggasoftware.indigo.gui;

import java.util.ArrayList;
import java.util.EventListener;
import java.util.EventObject;

abstract class MolEventSource
{
   public abstract void addListener(MolEventListener l);

   public abstract void removeListener(MolEventListener l);

   public abstract void fireEvent(EventObject o);

   public void alertListeners()
   {
      fireEvent(new EventObject(this));
   }
}

public class MolEvent extends MolEventSource
{
   ArrayList listeners = new ArrayList();
   public void addListener(MolEventListener l)
   {
     listeners.add(l);
   }
   
   public void removeListener(MolEventListener l)
   {
     listeners.remove(l);
   }
   
   public void fireEvent(EventObject o)
   {
     for (int i = 0; i < listeners.size(); i++)
     {
       MolEventListener l = (MolEventListener) listeners.get(i);
       l.handleEvent(o);
     }
   }
}
