/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * MoleculeOutputTable.java
 *
 * Created on Mar 5, 2011, 10:02:17 PM
 */
package com.epam.indigo.chemdiff;

import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.controls.*;
import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

/**
 *
 * @author achurinov
 */
public class MultiMoleculeOutputTable extends MoleculeOutputTable {

    private String _common_canonical_code;
    private CanonicalCodeGenerator _canonical_generator;

    public MultiMoleculeOutputTable() {
    }

    public void setCommonCanonicalCode(String code) {
        _common_canonical_code = code;
    }

    public void setCanonicalCodeGenerator(CanonicalCodeGenerator canonical_generator) {
        _canonical_generator = canonical_generator;
    }

    private void showSingleMolecule(RenderableObjectWithId item,
            boolean normalized, String canonical_code) {
        Frame parent = (Frame) getTopLevelAncestor();
        IndigoObject obj = item.getRenderableObject();
        if (obj == null) {
            String message = String.format("Exception:\n%s", item.getErrorMessageToRender());
            MessageBox.show(parent, message,
                    "Error during loading this molecule", MessageBox.ICON_ERROR);
            return;
        }
        if (normalized) {
            try {
                obj = _canonical_generator.createPreparedObject(obj);
            } catch (IndigoCheckedException ex) {
                MessageBox.show(parent, ex.getMessage(),
                        "Error during normalizing this molecule", MessageBox.ICON_ERROR);
                return;
            }
            try {
                obj.markEitherCisTrans();
            } catch (IndigoException ex) {
            }
        }
        // Show details window for single molecule
        SingleIndigoObjectWindow details = new SingleIndigoObjectWindow(parent,
                obj, item.getIndigoRenderer(), false);
        if (item.getErrorMessageToRender() != null) {
            details.setInformationMessage(item.getErrorMessageToRender());
        } else {
            details.setInformationMessage(canonical_code);
        }
        String title = item.getId(0);
        if (normalized) {
            title += " (normalized)";
        }
        details.setTitle(title);
        details.setVisible(true);

    }

    @Override
    protected void showMolecule(RenderableObjectWithId item) {
        Frame parent = (Frame) getTopLevelAncestor();
        MoleculeItem single = null;
        MultipleMoleculeItem mul_item = null;
        String canonical_code;
        if (item instanceof MoleculeItem) {
            single = (MoleculeItem) item;
            canonical_code = _common_canonical_code;
        } else {
            mul_item = (MultipleMoleculeItem) item;
            if (mul_item.isSingleMolecule()) {
                single = mul_item.getGroup(0).get(0);
            }
            canonical_code = mul_item.getCanonicalCode();
        }
        if (single != null) {
            showSingleMolecule(item, false, canonical_code);
        } else {
            // Show window with multiple molecules
            MultipleMoleculeWindow details = new MultipleMoleculeWindow(parent, mul_item);
            details.setCommonCanonicalCode(canonical_code);
            details.setRowHeight(getRowHeight());
            details.setCanonicalCodeGenerator(_canonical_generator);
            details.setVisible(true);
        }
    }

    @Override
    protected void addAdditionalItemsToPopup(JPopupMenu _popup_menu, TableCellMouseEvent evt) {
        final TableCellMouseEvent evt_final = evt;
        JMenuItem show_normalized_mi = new JMenuItem("Show normalized molecule");
        show_normalized_mi.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                RenderableObjectWithId item = _molecules.get(evt_final.row);
                String canonical_code;
                if (item instanceof MoleculeItem) {
                    canonical_code = _common_canonical_code;
                } else {
                    canonical_code = ((MultipleMoleculeItem) item).getCanonicalCode();
                }
                showSingleMolecule(item, true, canonical_code);
            }
        });
        _popup_menu.add(show_normalized_mi);
    }
}
