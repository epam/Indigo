/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.gga.indigo.chemdiff;

import com.gga.indigo.*;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.*;

import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.ClipboardOwner;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.StringSelection;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.Toolkit;
import java.io.*;

public class MolClicker extends MouseAdapter
{
    private int doubleClickDelay = 300;
    private Timer timer;
    private Indigo indigo;
    private IndigoRenderer indigo_renderer;

    public MolClicker( Indigo cur_indigo, IndigoRenderer cur_indigo_renderer )
    {
       indigo = cur_indigo;
       indigo_renderer = cur_indigo_renderer;

       ActionListener actionListener = new ActionListener()
       {
           @Override
           public void actionPerformed( ActionEvent e )
           {
              timer.stop();
           }
       };

       timer = new Timer(doubleClickDelay, actionListener);
       timer.setRepeats(false);
    }

    @Override
    public void mouseClicked( MouseEvent e )
    {
        ActionEvent ev = new ActionEvent( e.getSource(), e.getID(), e.paramString() );
        if (timer.isRunning())
        {
            timer.stop();
            fireSingleClick(ev);
            fireDoubleClick(ev);
        } else
        {
            timer.start();
            fireSingleClick(ev);
        }
    }

    protected void fireSingleClick( ActionEvent e )
    {
    }

    protected void fireDoubleClick( ActionEvent e )
    {
       JTable table = (JTable)e.getSource();

       int col = table.getSelectedColumn();
       int row = table.getSelectedRow();

       if (col == 0)
          return;

       MolCell mc = (MolCell)table.getValueAt(row, col);

       if (mc.object == null)
          return;

       JFrame cell_frame = new JFrame();
       cell_frame.setSize(new Dimension(510, 540));

       MolViewPanel mol_view = new MolViewPanel(indigo, indigo_renderer);
       mol_view.setImageSize(500, 500);

       IndigoObject molecule = mc.object.clone();

       try
       {
          indigo.indigoLayout(molecule.self);
       }
       catch( Exception ex )
       {
       }

       mol_view.setMol(molecule);

       String mol_name = molecule.name();

       try
       {
          mol_name = molecule.canonicalSmiles();
       }
       catch( Exception ex )
       {
          mol_name = null;
       }

       cell_frame.setTitle(mol_name);

       GroupLayout gl1 = new GroupLayout(cell_frame);

       gl1.setAutoCreateGaps(true);
       gl1.setHorizontalGroup(gl1.createParallelGroup(GroupLayout.Alignment.LEADING).
                addComponent(mol_view, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
       gl1.setVerticalGroup(gl1.createSequentialGroup().
                addComponent(mol_view, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));

       cell_frame.setVisible(true);
    }
}
