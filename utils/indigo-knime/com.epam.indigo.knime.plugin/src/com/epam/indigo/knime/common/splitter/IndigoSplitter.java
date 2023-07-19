package com.epam.indigo.knime.common.splitter;

import org.knime.core.data.DataRow;
import org.knime.core.node.BufferedDataContainer;

import com.epam.indigo.IndigoObject;

/**
 * This class should be used by splitters.
 * Running through a table's rows the class is supposed to distribute them among provided outputs.
 */
public abstract class IndigoSplitter {

   /**
    * This method estimates in which container the row to add and just add  it (using cell
    * from the row).
    * @param cons Containers to put the row
    * @param row A row to distribute
    * @param io IndigoObject for the target data cell
    */
   public abstract void distribute(BufferedDataContainer[] cons, DataRow row, IndigoObject io);
   
}
