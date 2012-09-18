package com.ggasoftware.indigo.controls;

import com.ggasoftware.indigo.IndigoObject;
import java.awt.Component;
import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutionException;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.SwingWorker;

public class MoleculesInputTable extends TitledBorderPanel {

    private ArrayList<MoleculeItem> _molecules = new ArrayList<MoleculeItem>();
    private boolean useProxyObjects = true;

    /**
     * Creates new form MoleculesInputTable
     */
    public MoleculesInputTable() {
        initComponents();
    }
    
    public boolean getUseProxyObjects ()
    {
        return useProxyObjects;
    }
    public void setUseProxyObjects (boolean use)
    {
        useProxyObjects = use;
    }

    public ArrayList<MoleculeItem> getMolecules() {
        return _molecules;
    }

    public void openLoadingDialog() {
        FileOpener fopener = new FileOpener();
        fopener.addExtension("sdf", "sd", "smi", "cml", "rdf", "mol");

        if (fopener.openFile("Open") == null) {
            return;
        }

        loadFile(fopener.getFile());
    }

    public void loadFile(final File f) {
        final ArrayList<MoleculeItem> new_molecules = new ArrayList<MoleculeItem>();
        //molecules_table.clear();

        Frame parent = (Frame) getTopLevelAncestor();

        final ProgressStatusDialog dlg = new ProgressStatusDialog(parent, true);
        dlg.setTitle("Loading...");

        final IndigoObjectsFileLoader loader = new IndigoObjectsFileLoader(Global.indigo, f);
        loader.setUseProxyObject(useProxyObjects);
        dlg.executeSwingWorker(loader);

        try {
            List<IndigoObjectWrapper> objects = loader.get();
            int added = 0;
            for (IndigoObjectWrapper obj : objects) {
                String id = String.format("Mol #%d", added);
                new_molecules.add(new MoleculeItem(obj, id));
                added++;
            }

            // Select ID field for the loaded molecules
            final SelectIDColumnDialog select_id_dlg =
                    new SelectIDColumnDialog(parent, new_molecules, true, true);
            select_id_dlg.setVisible(true);

            if (select_id_dlg.isCanceled()) {
                return;
            }

            // Update molecules properties
            SwingWorker<Void, Void> update_properties_worker = new SwingWorker<Void, Void>() {

                @Override
                protected Void doInBackground() throws Exception {
                    int processed = 0;
                    int errors_count = 0;
                    for (MoleculeItem obj : new_molecules) {
                        boolean set_serial_on_error = false;
                        try {
                            // TODO: Remove bug!
                            // 1. load valence_test
                            // 2. load test
                            // 3. Error!
                            obj.setId(select_id_dlg.getMoleculeID(obj, processed));
                        } catch (IndigoCheckedException ex) {
                            set_serial_on_error = true;
                        }

                        if (set_serial_on_error) {
                            // Set serial number
                            obj.setId(select_id_dlg.getSerialNumber(processed));
                            errors_count++;
                        }
                        processed++;
                        setProgress(100 * processed / new_molecules.size());
                    }
                    dlg.setStepName("Adding molecules to the table");
                    _molecules.addAll(new_molecules);
                    molecules_with_id_table.setObjects(_molecules);

                    StringBuilder subtitle = new StringBuilder();
                    subtitle.append(String.format(": %d molecule%s", _molecules.size(),
                            _molecules.size() != 1 ? "s" : ""));
                    if (errors_count != 0) {
                        subtitle.append(String.format(" (with %d not valid)", errors_count));
                    }
                    setSubtitle(subtitle.toString());

                    String file_name = f.getAbsolutePath();
                    if (file_name_field.getText().length() > 0) {
                        file_name = "multiple files";
                    }
                    file_name_field.setText(file_name);
                    file_name_field.setCaretPosition(file_name.length());
                    return null;
                }
            };
            dlg.setTitle("Reading molecule properties...");
            dlg.executeSwingWorker(update_properties_worker);
        } catch (InterruptedException ex) {
            System.err.println(">>>>" + ex.getMessage());
            ex.printStackTrace();
        } catch (ExecutionException ex) {
            System.err.println(">>>>" + ex.getMessage());
            ex.printStackTrace();
            //Logger.getLogger(TestFrame.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    public void setRowHeight(int height) {
        molecules_with_id_table.setRowHeight(height);
    }

    public int getRowHeight() {
        return molecules_with_id_table.getRowHeight();
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        molecules_with_id_table = new com.ggasoftware.indigo.controls.MoleculeTableWithIdPanel();
        jPanel1 = new javax.swing.JPanel();
        add_button = new javax.swing.JButton();
        clear_button = new javax.swing.JButton();
        file_name_field = new javax.swing.JTextField();

        molecules_with_id_table.setIdColumnCount(1);
        molecules_with_id_table.addTableCellMouseListener(new com.ggasoftware.indigo.controls.TableCellMouseListener() {
            public void cellMouseDoubleClick(com.ggasoftware.indigo.controls.TableCellMouseEvent evt) {
                molecules_with_id_tableCellMouseDoubleClick(evt);
            }
            public void cellShowPopupMenu(com.ggasoftware.indigo.controls.TableCellMouseEvent evt) {
                molecules_with_id_tableCellShowPopupMenu(evt);
            }
        });

        jPanel1.setLayout(new javax.swing.BoxLayout(jPanel1, javax.swing.BoxLayout.LINE_AXIS));

        add_button.setText("Add");
        add_button.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                add_buttonActionPerformed(evt);
            }
        });
        jPanel1.add(add_button);

        clear_button.setText("Clear");
        clear_button.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                clear_buttonActionPerformed(evt);
            }
        });
        jPanel1.add(clear_button);

        file_name_field.setEditable(false);
        jPanel1.add(file_name_field);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(molecules_with_id_table, javax.swing.GroupLayout.DEFAULT_SIZE, 346, Short.MAX_VALUE)
            .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(molecules_with_id_table, javax.swing.GroupLayout.DEFAULT_SIZE, 365, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents

   private void add_buttonActionPerformed (java.awt.event.ActionEvent evt)//GEN-FIRST:event_add_buttonActionPerformed
   {//GEN-HEADEREND:event_add_buttonActionPerformed
       openLoadingDialog();
}//GEN-LAST:event_add_buttonActionPerformed

   private void molecules_with_id_tableCellMouseDoubleClick (com.ggasoftware.indigo.controls.TableCellMouseEvent evt)//GEN-FIRST:event_molecules_with_id_tableCellMouseDoubleClick
   {//GEN-HEADEREND:event_molecules_with_id_tableCellMouseDoubleClick
       MoleculeItem item = _molecules.get(evt.row);

       Frame parent = (Frame) getTopLevelAncestor();

       IndigoObject obj = item.getRenderableObject();
       if (obj == null) {
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
       show_mi.addActionListener(new ActionListener() {

           public void actionPerformed(ActionEvent e) {
               molecules_with_id_tableCellMouseDoubleClick(evt_final);
           }
       });
       _popup_menu.add(show_mi);

       _popup_menu.show((Component) evt.mouse_event.getSource(), evt.mouse_event.getX(), evt.mouse_event.getY());
   }//GEN-LAST:event_molecules_with_id_tableCellShowPopupMenu

    public void clear() {
        _molecules.clear();
        molecules_with_id_table.setObjects(_molecules);
        file_name_field.setText("");
    }

    private void clear_buttonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_clear_buttonActionPerformed
        clear();
    }//GEN-LAST:event_clear_buttonActionPerformed
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton add_button;
    private javax.swing.JButton clear_button;
    private javax.swing.JTextField file_name_field;
    private javax.swing.JPanel jPanel1;
    private com.ggasoftware.indigo.controls.MoleculeTableWithIdPanel molecules_with_id_table;
    // End of variables declaration//GEN-END:variables
}
