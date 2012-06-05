package com.ggasoftware.indigo.legio;

import com.ggasoftware.indigo.controls.MessageBox;
import java.io.PrintWriter;
import java.io.StringWriter;
import javax.swing.UIManager;

public class Main {

    public static void main(String[] args) {
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            System.out.println("Error setting native LAF: " + e);
        }
        
        try {
            MainFrame mf = new MainFrame();
            mf.setLocationRelativeTo(null);
            mf.setVisible(true);
        } catch (Throwable err) {
            StringWriter sw = new StringWriter();
            err.printStackTrace(new PrintWriter(sw));
            String error_as_string = sw.toString();

            MessageBox.show(null, error_as_string, "Error", MessageBox.ICON_ERROR);
            System.exit(0);
        }
    }
}
