/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.gui;

import java.util.ArrayList;
import java.util.List;

/**
 * 
 * @author rybalkin
 */
public class IndigoEventSource<EventData>
{
   private List< IndigoEventListener<EventData> > listener_list = 
           new ArrayList<IndigoEventListener<EventData>>();
   private Object source;
   
   public IndigoEventSource (Object source)
   {
      this.source = source;
   }
   
   public void addListener(IndigoEventListener<EventData> listener)
   {
      listener_list.add(listener);
   }
    
   public void removeListener(IndigoEventListener<EventData> listener)
   {
      listener_list.remove(listener);
   }
   
   public void fireEvent (EventData object)
   {
     for (IndigoEventListener<EventData> listener : listener_list)
        listener.handleEvent(source, object);
   }
}
