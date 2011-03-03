/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.gui;

/**
 *
 * @author rybalkin
 */
public interface IndigoEventListener<EventData>
{
   void handleEvent(Object source, EventData event);
}
