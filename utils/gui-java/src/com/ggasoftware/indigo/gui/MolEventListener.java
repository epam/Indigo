/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware.indigo.gui;

import java.util.EventListener;
import java.util.EventObject;

public abstract class MolEventListener implements EventListener
{
   public abstract void handleEvent(EventObject o);
}
