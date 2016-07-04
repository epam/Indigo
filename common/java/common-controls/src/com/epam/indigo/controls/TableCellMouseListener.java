package com.epam.indigo.controls;

import java.util.EventListener;

public interface TableCellMouseListener extends EventListener
{
   public void cellMouseDoubleClick (TableCellMouseEvent event);
   public void cellShowPopupMenu (TableCellMouseEvent cme);
}
