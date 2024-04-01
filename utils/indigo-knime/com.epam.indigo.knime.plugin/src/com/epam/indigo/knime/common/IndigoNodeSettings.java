package com.epam.indigo.knime.common;

import java.util.ArrayList;

import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JFormattedTextField;
import javax.swing.JRadioButton;
import javax.swing.JSpinner;
import javax.swing.JTextField;

import org.knime.core.data.DataTableSpec;
import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;
import org.knime.core.node.NotConfigurableException;
import org.knime.core.node.defaultnodesettings.SettingsModel;
import org.knime.core.node.defaultnodesettings.SettingsModelBoolean;
import org.knime.core.node.defaultnodesettings.SettingsModelDouble;
import org.knime.core.node.defaultnodesettings.SettingsModelInteger;
import org.knime.core.node.defaultnodesettings.SettingsModelString;
import org.knime.core.node.port.PortObjectSpec;
import org.knime.core.node.util.ColumnSelectionComboxBox;

import com.epam.indigo.knime.cell.IndigoMolValue;
import com.epam.indigo.knime.cell.IndigoQueryMolValue;
import com.epam.indigo.knime.cell.IndigoQueryReactionValue;
import com.epam.indigo.knime.cell.IndigoReactionValue;

/**
 * Class for basic settings handling
 */
public class IndigoNodeSettings {
   
   private interface DialogMap {
      abstract void load(final DataTableSpec[] specs) throws NotConfigurableException;
      abstract void save();
   }
   
   private class CheckDialogMap implements DialogMap{
      private final JCheckBox chbDialogComp;
      private final SettingsModelBoolean mapParam;
      
      public CheckDialogMap(JCheckBox dialogComp, SettingsModelBoolean mapParam) {
         chbDialogComp = dialogComp;
         this.mapParam = mapParam;
      }
      public void load(DataTableSpec[] specs) {
         chbDialogComp.setSelected(mapParam.getBooleanValue());
      }
      public void save() {
         mapParam.setBooleanValue(chbDialogComp.isSelected());
      }
      
   }
   private class RadioDialogMap implements DialogMap {
      private final JRadioButton rbDialogComp;
      private final SettingsModelBoolean mapParam;
      
      public RadioDialogMap(JRadioButton dialogComp, SettingsModelBoolean mapParam) {
         rbDialogComp = dialogComp;
         this.mapParam = mapParam;
      }
      public void load(DataTableSpec[] specs) {
         rbDialogComp.setSelected(mapParam.getBooleanValue());
      }
      public void save() {
         mapParam.setBooleanValue(rbDialogComp.isSelected());
      }
      
   }
   private class ComboDialogMap implements DialogMap {
      
      private final JComboBox<?> chbDialogComp;
      private final SettingsModelInteger mapParam;

      public ComboDialogMap(JComboBox<?> dialogComp, SettingsModelInteger mapParam) {
         this.chbDialogComp = dialogComp;
         this.mapParam = mapParam;
      }
      public void load(DataTableSpec[] specs) {
         chbDialogComp.setSelectedIndex(mapParam.getIntValue());
      }
      public void save() {
         mapParam.setIntValue(chbDialogComp.getSelectedIndex());
      }
      
   }
   private class ComboStringDialogMap implements DialogMap {
      
      private final JComboBox<?> chbDialogComp;
      private final SettingsModelString mapParam;

      public ComboStringDialogMap(JComboBox<?> dialogComp, SettingsModelString mapParam) {
         this.chbDialogComp = dialogComp;
         this.mapParam = mapParam;
      }
      public void load(DataTableSpec[] specs) {
         String value = mapParam.getStringValue(); 
         for (int i = 0; i < chbDialogComp.getItemCount(); i++)
            if (chbDialogComp.getItemAt(i).toString().equals(value)) {
               chbDialogComp.setSelectedIndex(i);
               break;
            }
      }
      public void save() {
         mapParam.setStringValue(chbDialogComp.getSelectedItem().toString());
      }
      
   }
   
   private class StringDialogMap implements DialogMap {
      private final JTextField txtDialogComp;
      private final SettingsModelString mapParam;

      public StringDialogMap(JTextField dialogComp, SettingsModelString mapParam) {
         this.txtDialogComp = dialogComp;
         this.mapParam = mapParam;
      }
      public void load(DataTableSpec[] specs) {
         txtDialogComp.setText(mapParam.getStringValue());
      }
      public void save() {
         mapParam.setStringValue(txtDialogComp.getText());
      }

   }
   
   private class SpinnerDialogMap implements DialogMap {

      private final JSpinner spnDialogComp;
      private final SettingsModelInteger mapParam;
      public SpinnerDialogMap(JSpinner dialogComp, SettingsModelInteger mapParam) {
         this.spnDialogComp = dialogComp;
         this.mapParam = mapParam;
      }
      public void load(DataTableSpec[] specs) {
         spnDialogComp.setValue(mapParam.getIntValue());
      }
      public void save() {
         mapParam.setIntValue((Integer)spnDialogComp.getValue());
      }
   }
   
   private class ColumnDialogMap implements DialogMap {

      private final ColumnSelectionComboxBox dialogComp;
      private final int specPort;
      private final SettingsModelString mapParam;
      private boolean optional = false;
      private boolean disabled = false;

      public ColumnDialogMap(ColumnSelectionComboxBox dialogComp, int specPort, SettingsModelString mapParam) {
         this.dialogComp = dialogComp;
         this.specPort = specPort;
         this.mapParam = mapParam;
      }
      public void load(DataTableSpec[] specs) throws NotConfigurableException {
         disabled = (optional && specs[specPort] == null);
         if (disabled) {
            return;
         }
         dialogComp.update(specs[specPort],   mapParam.getStringValue());
      }
      public void save() {
         if (disabled) {
            return;
         }
         mapParam.setStringValue(dialogComp.getSelectedColumn());
      }
      public void setOptional (boolean optional) {
         this.optional = optional;
      }
   }
   
   private class DoubleDialogMap implements DialogMap {

      private final JFormattedTextField txtDialogComp;
      private final SettingsModelDouble mapParam;

      public DoubleDialogMap(JFormattedTextField dialogComp, SettingsModelDouble mapParam) {
         this.txtDialogComp = dialogComp;
         this.mapParam = mapParam;
      }
      public void load(DataTableSpec[] specs) throws NotConfigurableException {
         txtDialogComp.setValue(mapParam.getDoubleValue());
      }
      public void save() {
         mapParam.setDoubleValue(((Number)txtDialogComp.getValue()).doubleValue());
      }

   }
   
   public class FloatDialogMap implements DialogMap {
      private final JFormattedTextField txtDialogComp;
      private final SettingsModelFloat mapParam;
      
      public FloatDialogMap(JFormattedTextField dialogComp, SettingsModelFloat mapParam) {
         txtDialogComp = dialogComp;
         this.mapParam = mapParam;
      }

      @Override
      public void load(DataTableSpec[] specs) throws NotConfigurableException {
         txtDialogComp.setValue(mapParam.getFloatValue());
      }

      @Override
      public void save() {
         mapParam.setFloatValue(((Number)txtDialogComp.getValue()).floatValue());
      }

   }
   
   public class DeprecatedSettingsModelBooleanInverse extends SettingsModelBoolean {

      private final String configName;
      private final boolean defaultValue;

      public DeprecatedSettingsModelBooleanInverse(String configName,
            boolean defaultValue) {
         super(configName, defaultValue);
         this.configName = configName;
         this.defaultValue = defaultValue;
      }
      
      @Override
      protected void loadSettingsForModel(NodeSettingsRO settings)
            throws InvalidSettingsException {
         setBooleanValue(!settings.getBoolean(configName));
      }
      
      @Override
      protected void loadSettingsForDialog(NodeSettingsRO settings,
            PortObjectSpec[] specs) throws NotConfigurableException {
         setBooleanValue(!settings.getBoolean(configName, defaultValue));
      }
      
      @Override
      protected void saveSettingsForModel(NodeSettingsWO settings) {
         settings.addBoolean(configName, !getBooleanValue());
      }
   }
   
   private final ArrayList<SettingsModel> allSettings = new ArrayList<SettingsModel>();
   private final ArrayList<DialogMap> allDialogSettings = new ArrayList<DialogMap>();
   public String warningMessage;
   
   
   protected void addSettingsParameter(SettingsModel param) {
      allSettings.add(param);
   }
   
   public void loadSettingsFrom(NodeSettingsRO settings) throws InvalidSettingsException {
      loadSettingsFrom(settings, false);
   }
   
   public void loadSettingsFrom(NodeSettingsRO settings, boolean throwError) throws InvalidSettingsException {
      warningMessage = null;
      StringBuilder wMessage = new StringBuilder();
      
      for (SettingsModel param : allSettings) {
         try {
            param.loadSettingsFrom(settings);
         } catch (InvalidSettingsException e) {
            wMessage.append(e.getMessage());
            wMessage.append('\n');
         }
      }
      loadAdditionalSettings(settings);
      if(wMessage.length() > 0 ) {
         wMessage.insert(0, "Not all the settings have been loaded: ");
         wMessage.append("Probably, these settings are new, and the default values have been used. Please, review the configuration and resave it.");
         warningMessage = wMessage.toString();
         if(throwError)
            throw new InvalidSettingsException(warningMessage);
      }
   }

   public void saveSettingsTo(NodeSettingsWO settings) {
      for (SettingsModel param : allSettings) {
         param.saveSettingsTo(settings);
      }
      saveAdditionalSettings(settings);
   }

   /*
    * Additional settings processing
    */
   protected void loadAdditionalSettings(NodeSettingsRO settings) throws InvalidSettingsException{
   }
   protected void saveAdditionalSettings(NodeSettingsWO settings) {
   }
   public void validateAdditionalSettings(NodeSettingsRO settings) throws InvalidSettingsException {
   }
   
   
   public void registerDialogComponent(JCheckBox dialogComp, SettingsModelBoolean mapParam) {
      allDialogSettings.add(new CheckDialogMap(dialogComp, mapParam));
   }
   
   public void registerDialogComponent(JRadioButton dialogComp, SettingsModelBoolean mapParam) {
      allDialogSettings.add(new RadioDialogMap(dialogComp, mapParam));
   }
   
   public void registerDialogComponent(JComboBox<?> dialogComp, SettingsModelInteger mapParam) {
      allDialogSettings.add(new ComboDialogMap(dialogComp, mapParam));
   }

   public void registerDialogComponent(JComboBox<?> dialogComp, SettingsModelString mapParam) {
      allDialogSettings.add(new ComboStringDialogMap(dialogComp, mapParam));
   }
   
   public void registerDialogComponent(JTextField dialogComp, SettingsModelString mapParam) {
      allDialogSettings.add(new StringDialogMap(dialogComp, mapParam));
   }
   
   public void registerDialogComponent(JSpinner dialogComp, SettingsModelInteger mapParam) {
      allDialogSettings.add(new SpinnerDialogMap(dialogComp, mapParam));
   }
   
   public void registerDialogComponent(ColumnSelectionComboxBox dialogComp, int specPort, SettingsModelString mapParam) {
      registerDialogComponent(dialogComp, specPort, mapParam, false);
   }

   public void registerDialogComponent(ColumnSelectionComboxBox dialogComp, int specPort, SettingsModelString mapParam, boolean optional) {
      ColumnDialogMap columnDialogMap = new ColumnDialogMap(dialogComp, specPort, mapParam);
      columnDialogMap.setOptional(optional);
      allDialogSettings.add(columnDialogMap);
   }
   
   public void registerDialogComponent(JFormattedTextField dialogComp, SettingsModelDouble mapParam) {
      allDialogSettings.add(new DoubleDialogMap(dialogComp, mapParam));
   }
   
   public void registerDialogComponent(JFormattedTextField dialogComp,
         SettingsModelFloat mapParam) {
      allDialogSettings.add(new FloatDialogMap(dialogComp, mapParam));
      
   }
   
   public void loadDialogSettings(final DataTableSpec[] specs) throws NotConfigurableException {
      for(DialogMap dialogMap : allDialogSettings) {
         dialogMap.load(specs);
      }
   }
   public void saveDialogSettings() {
      for(DialogMap dialogMap: allDialogSettings) {
         dialogMap.save();
      }
   }
   
   public enum STRUCTURE_TYPE {
      Reaction, Molecule, Unknown;
   }
   

   
   /*
    * Returns current column selection state
    */
   public static STRUCTURE_TYPE getStructureType(DataTableSpec tSpec, DataTableSpec qSpec, String tName, String qName) {
      STRUCTURE_TYPE result = STRUCTURE_TYPE.Unknown;
      
      int reactions = 0;
      int molecules = 0;
      
      if(tSpec != null) {
         if(tSpec.containsName(tName))
            if(tSpec.getColumnSpec(tName).getType().isCompatible(IndigoReactionValue.class))
               ++reactions;
            else if (tSpec.getColumnSpec(tName).getType().isCompatible(IndigoMolValue.class)) 
               ++molecules;
      }
      
      if(qSpec != null) {
         if(qSpec.containsName(qName))
            if(qSpec.getColumnSpec(qName).getType().isCompatible(IndigoQueryReactionValue.class))
               ++reactions;
            else if(qSpec.getColumnSpec(qName).getType().isCompatible(IndigoQueryMolValue.class))
               ++molecules;
      }
      if(reactions == 2)
         result = STRUCTURE_TYPE.Reaction;
      else if(molecules == 2)
         result = STRUCTURE_TYPE.Molecule;
      
      return result;
   }
   
   public static int searchColumnIdx(String colName, String errorMsg, DataTableSpec inPortSpec) {
      int colIdx = -1;

      if (colName == null)
         throw new RuntimeException(errorMsg + " column not found");

      colIdx = inPortSpec.findColumnIndex(colName);

      if (colIdx == -1)
         throw new RuntimeException(errorMsg + " column not found");
      
      return colIdx;
   }

   public static STRUCTURE_TYPE getStructureType(DataTableSpec tSpec, String tName) {
      STRUCTURE_TYPE result = STRUCTURE_TYPE.Unknown;

      if (tSpec != null) {
         if (tSpec.containsName(tName))
            if (tSpec.getColumnSpec(tName).getType().isCompatible(IndigoReactionValue.class))
               result = STRUCTURE_TYPE.Reaction;
            else if (tSpec.getColumnSpec(tName).getType().isCompatible(IndigoMolValue.class))
               result = STRUCTURE_TYPE.Molecule;
      }
      return result;
   }
   
}
