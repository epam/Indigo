package com.epam.indigo.knime.io.rxnreader;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

import org.knime.chem.types.RxnCell;
import org.knime.chem.types.RxnCellFactory;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataColumnSpec;
import org.knime.core.data.DataColumnSpecCreator;
import org.knime.core.data.DataTableSpec;
import org.knime.core.data.RowKey;
import org.knime.core.data.def.DefaultRow;
import org.knime.core.node.BufferedDataContainer;
import org.knime.core.node.BufferedDataTable;
import org.knime.core.node.CanceledExecutionException;
import org.knime.core.node.ExecutionContext;
import org.knime.core.node.ExecutionMonitor;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeModel;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;

public class RXNReaderNodeModel extends NodeModel {

   private RXNReaderSettings nodeSettings = new RXNReaderSettings();

   protected RXNReaderNodeModel() {
      super(0, 1);
   }

   @Override
   protected void loadInternals(File nodeInternDir, ExecutionMonitor exec)
   throws IOException, CanceledExecutionException {
   }

   @Override
   protected void saveInternals(File nodeInternDir, ExecutionMonitor exec)
   throws IOException, CanceledExecutionException {
   }

   @Override
   protected void saveSettingsTo(NodeSettingsWO settings) {
      nodeSettings.saveSettings(settings);
   }

   @Override
   protected void validateSettings(NodeSettingsRO settings)
   throws InvalidSettingsException {
      // TODO Auto-generated method stub

   }

   @Override
   protected void loadValidatedSettingsFrom(NodeSettingsRO settings)
   throws InvalidSettingsException {
      nodeSettings.loadSettings(settings);
   }

   @Override
   protected void reset() {
   }

   @Override
   protected DataTableSpec[] configure(DataTableSpec[] inSpecs)
   throws InvalidSettingsException {
      // TODO Auto-generated method stub

      return new DataTableSpec[] {
            new DataTableSpec(new DataColumnSpec[] {
                  new DataColumnSpecCreator("Reaction", RxnCell.TYPE).createSpec()
            })
      };
   }

   @Override
   protected BufferedDataTable[] execute(BufferedDataTable[] inData, ExecutionContext exec) throws Exception {
      BufferedDataContainer outputContainer = exec.createDataContainer(configure(null)[0]);
      
      int rowKey = 0;
      BufferedReader reader = null;
      try {
         reader = new BufferedReader(new FileReader(nodeSettings.fileName));
         String line = reader.readLine(); // $RXN or $RDFILE
         if (line.startsWith("$RXN")) {
            StringBuffer fileData = new StringBuffer();
            do {
               fileData.append(line).append("\n");
            } while ((line = reader.readLine()) != null);
            outputContainer.addRowToTable(new DefaultRow(
                  new RowKey(String.valueOf(rowKey)),
                  new DataCell[] { RxnCellFactory.create(fileData.toString()) }
                  ));
         } else if (line.startsWith("$RDFILE")) {
            line = reader.readLine(); // $DATM
            while (null != (line = reader.readLine())) {
               if (line.toUpperCase().startsWith("$RFMT") || line.toUpperCase().startsWith("$PCRXN")) {
                  StringBuffer fileData = new StringBuffer();
                  if (null == (line = reader.readLine())) { // $RXN
                     throw new Exception("Unexpected end of file.");
                  }
                  fileData.append(line).append("\n");
                  if (null == (line = reader.readLine())) { // blank line
                     throw new Exception("Unexpected end of file.");
                  }
                  fileData.append(line).append("\n");
                  if (null == (line = reader.readLine())) { // created by
                     throw new Exception("Unexpected end of file.");
                  }
                  fileData.append(line).append("\n");
                  if (null == (line = reader.readLine())) { // comments
                     throw new Exception("Unexpected end of file.");
                  }
                  fileData.append(line).append("\n");
                  if (null == (line = reader.readLine())) { // counters
                     throw new Exception("Unexpected end of file.");
                  }
                  fileData.append(line).append("\n");
                  for (int i = Integer.valueOf(line.substring(0, 3).trim()) + Integer.valueOf(line.substring(3, 6).trim()); i > 0; i--) {
                     do {
                        if (null == (line = reader.readLine())) {
                           throw new Exception("Unexpected end of file.");
                        }
                        fileData.append(line).append("\n");
                     } while (!line.startsWith("M  END"));
                  }
                  outputContainer.addRowToTable(new DefaultRow(
                        new RowKey(String.valueOf(rowKey++)),
                        new DataCell[] { RxnCellFactory.create(fileData.toString()) }
                        ));
               } else if (line.toUpperCase().startsWith("$RIREG") || line.toUpperCase().startsWith("$REREG")) {
                  // just skipping it at the moment....
               }
            };
         } else {
            throw new Exception("Bad file format.");
         }
      } finally {
         if (reader != null) {
            reader.close();
         } else {
            throw new Exception("Cannot read file.");
         }
      }
      

      outputContainer.close();

      return new BufferedDataTable[] { outputContainer.getTable() };
   }
}
