package com.ggasoftware.indigo.controls;

import javax.swing.JPanel;

public class BeanBase extends JPanel {
   protected String _name;
   protected boolean _deceased;

   public BeanBase() {
   }

   public String getName() {
      return (this._name);
   }
   public void setName(String name) {
      this._name = name;
   }
   public boolean isDeceased() {
      return (this._deceased);
   }
   public void setDeceased(boolean deceased) {
      this._deceased = deceased;
   }
}
