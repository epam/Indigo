package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.IndigoObject;
import com.ggasoftware.indigo.controls.*;
import java.awt.Component;
import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutionException;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JTextArea;
import javax.swing.SwingWorker;

public class InputTable extends TitledBorderPanel
{
   private ArrayList<MoleculeItem> _molecules = new ArrayList<MoleculeItem>();

   /** Creates new form InputTable */
   public InputTable ()
   {
      initComponents();
   }

   public ArrayList<MoleculeItem> getMolecules ()
   {
      return _molecules;
   }
   
   public void openLoadingDialog ()
   {
      FileOpener fopener = new FileOpener();
      fopener.addExtension("sdf", "sd", "smi", "cml", "rdf", "mol");

      if (fopener.openFile("Open") == null)
         return;

      loadFile(fopener.getFile());
   }
   
   public void loadFile (final File f)
   {
      final ArrayList<MoleculeItem> new_molecules = new ArrayList<MoleculeItem>();
      //molecules_table.clear();
      
      Frame parent = (Frame)getTopLevelAncestor();
      
      final ProgressStatusDialog dlg = new ProgressStatusDialog(parent, true);
      dlg.setTitle("Loading...");
      
      final IndigoObjectsFileLoader loader = new IndigoObjectsFileLoader(Global.indigo, f);
      dlg.executeSwingWorker(loader);
     
      try
      {
         List<IndigoObjectWrapper> objects = loader.get();
         int added = 0;
         for (IndigoObjectWrapper obj : objects)
         {
            String id = String.format("Mol #%d", added);
            new_molecules.add(new MoleculeItem(obj, id));
            added++;
         }

         // Select ID field for the loaded molecules
         final SelectIDColumnDialog select_id_dlg = 
                 new SelectIDColumnDialog(parent, new_molecules, true, true);
         select_id_dlg.setVisible(true);
         
         if (select_id_dlg.isCanceled())
            return;
         
         // Update molecules properties
         SwingWorker<Void, Void> update_properties_worker = new SwingWorker<Void, Void>()
         {
            @Override
            protected Void doInBackground () throws Exception
            {
               int processed = 0;
               int errors_count = 0;
               for (MoleculeItem obj : new_molecules)
               {
                  boolean set_serial_on_error = false;
                  try 
                  {
                     // TODO: Remove bug!
                     // 1. load valence_test
                     // 2. load test
                     // 3. Error!
                     obj.setId(select_id_dlg.getMoleculeID(obj, processed));
                  }
                  catch (IndigoCheckedException ex)
                  {
                     set_serial_on_error = true;
                  }
                  
                  if (set_serial_on_error)
                  {
                     // Set serial number
                     obj.setId(select_id_dlg.getSerialNumber(processed));
                     errors_count++;
                  }
                  processed++;
                  setProgress(100 * processed / new_molecules.size());
               }
               dlg.setStepName("Adding molecules to the table");
               molecules_with_id_table.setObjects(new_molecules);

               StringBuilder subtitle = new StringBuilder();
               subtitle.append(String.format(": %d molecule%s", new_molecules.size(),
                       new_molecules.size() > 1 ? "s" : ""));
               if (errors_count != 0)
                  subtitle.append(String.format(" (with %d not valid)", errors_count));
               setSubtitle(subtitle.toString());
               
               _molecules = new_molecules;
               
               String file_name = f.getAbsolutePath();
               file_name_field.setText(file_name);
               file_name_field.setCaretPosition(file_name.length());
               return null;
            }
         };
         dlg.setTitle("Reading molecule properties...");
         dlg.executeSwingWorker(update_properties_worker);
      }
      catch (InterruptedException ex)
      {
         System.err.println(">>>>" + ex.getMessage());
         ex.printStackTrace();
      }
      catch (ExecutionException ex)
      {
         System.err.println(">>>>" + ex.getMessage());
         ex.printStackTrace();
         //Logger.getLogger(TestFrame.class.getName()).log(Level.SEVERE, null, ex);
      }
   }

   public void setRowHeight (int height)
   {
      molecules_with_id_table.setRowHeight(height);
   }

   public int getRowHeight ()
   {
      return molecules_with_id_table.getRowHeight();
   }
   
   /** This method is called from within the constructor to
    * initialize the form.
    * WARNING: Do NOT modify this code. The content of this method is
    * always regenerated by the Form Editor.
    */
   @SuppressWarnings("unchecked")
   // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
   private void initComponents() {

      load_button_panel = new javax.swing.JPanel();
      load_button = new javax.swing.JButton();
      file_name_field = new javax.swing.JTextField();
      molecules_with_id_table = new com.ggasoftware.indigo.chemdiff.MoleculeTableWithIdPanel();

      load_button.setText("Load molecules");
      load_button.addActionListener(new java.awt.event.ActionListener() {
         public void actionPerformed(java.awt.event.ActionEvent evt) {
            load_buttonActionPerformed(evt);
         }
      });

      file_name_field.setEditable(false);

      javax.swing.GroupLayout load_button_panelLayout = new javax.swing.GroupLayout(load_button_panel);
      load_button_panel.setLayout(load_button_panelLayout);
      load_button_panelLayout.setHorizontalGroup(
         load_button_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, load_button_panelLayout.createSequentialGroup()
            .addComponent(file_name_field, javax.swing.GroupLayout.DEFAULT_SIZE, 209, Short.MAX_VALUE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(load_button, javax.swing.GroupLayout.PREFERRED_SIZE, 115, javax.swing.GroupLayout.PREFERRED_SIZE))
      );
      load_button_panelLayout.setVerticalGroup(
         load_button_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGroup(load_button_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
            .addComponent(load_button, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(file_name_field, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
      );

      molecules_with_id_table.setIdColumnCount(1);
      molecules_with_id_table.addTableCellMouseListener(new com.ggasoftware.indigo.controls.TableCellMouseListener() {
         public void cellMouseDoubleClick(com.ggasoftware.indigo.controls.TableCellMouseEvent evt) {
            molecules_with_id_tableCellMouseDoubleClick(evt);
         }
         public void cellShowPopupMenu(com.ggasoftware.indigo.controls.TableCellMouseEvent evt) {
            molecules_with_id_tableCellShowPopupMenu(evt);
         }
      });

      javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
      this.setLayout(layout);
      layout.setHorizontalGroup(
         layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 330, Short.MAX_VALUE)
         .addComponent(load_button_panel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
         .addComponent(molecules_with_id_table, javax.swing.GroupLayout.DEFAULT_SIZE, 330, Short.MAX_VALUE)
      );
      layout.setVerticalGroup(
         layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
         .addGap(0, 391, Short.MAX_VALUE)
         .addGroup(layout.createSequentialGroup()
            .addComponent(load_button_panel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(molecules_with_id_table, javax.swing.GroupLayout.DEFAULT_SIZE, 362, Short.MAX_VALUE))
      );
   }// </editor-fold>//GEN-END:initComponents

   private void load_buttonActionPerformed (java.awt.event.ActionEvent evt)//GEN-FIRST:event_load_buttonActionPerformed
   {//GEN-HEADEREND:event_load_buttonActionPerformed
      openLoadingDialog ();
}//GEN-LAST:event_load_buttonActionPerformed

   private void molecules_with_id_tableCellMouseDoubleClick (com.ggasoftware.indigo.controls.TableCellMouseEvent evt)//GEN-FIRST:event_molecules_with_id_tableCellMouseDoubleClick
   {//GEN-HEADEREND:event_molecules_with_id_tableCellMouseDoubleClick
      MoleculeItem item = _molecules.get(evt.row);
      
      Frame parent = (Frame)getTopLevelAncestor();
      
      IndigoObject obj = item.getRenderableObject();
      if (obj == null)
      {
         String message = String.format("Exception:\n%s", item.getErrorMessageToRender());
         MessageBox.show(parent, message, 
                 "Error during loading this molecule", MessageBox.ICON_ERROR);
         return;
      }
      
      SingleIndigoObjectWindow details = new SingleIndigoObjectWindow(parent, 
              obj, item.getIndigoRenderer(), false);
      details.setInformationMessage(item.getErrorMessageToRender());
      details.setTitle(item.getId());
      details.setVisible(true);
   }//GEN-LAST:event_molecules_with_id_tableCellMouseDoubleClick

   private void molecules_with_id_tableCellShowPopupMenu (com.ggasoftware.indigo.controls.TableCellMouseEvent evt)//GEN-FIRST:event_molecules_with_id_tableCellShowPopupMenu
   {//GEN-HEADEREND:event_molecules_with_id_tableCellShowPopupMenu
      final TableCellMouseEvent evt_final = evt;
      
      JPopupMenu _popup_menu = new JPopupMenu();
      JMenuItem show_mi = new JMenuItem("Open in a new window");
      show_mi.addActionListener(new ActionListener()
      {
         public void actionPerformed (ActionEvent e)
         {
            molecules_with_id_tableCellMouseDoubleClick(evt_final);
         }
      });
      _popup_menu.add(show_mi);
      
      _popup_menu.show((Component)evt.mouse_event.getSource(), evt.mouse_event.getX(), evt.mouse_event.getY());
   }//GEN-LAST:event_molecules_with_id_tableCellShowPopupMenu

   // Variables declaration - do not modify//GEN-BEGIN:variables
   private javax.swing.JTextField file_name_field;
   private javax.swing.JButton load_button;
   private javax.swing.JPanel load_button_panel;
   private com.ggasoftware.indigo.chemdiff.MoleculeTableWithIdPanel molecules_with_id_table;
   // End of variables declaration//GEN-END:variables
}
