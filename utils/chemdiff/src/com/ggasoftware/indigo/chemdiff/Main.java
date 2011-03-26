package com.ggasoftware.indigo.chemdiff;

import com.ggasoftware.indigo.Indigo;
import com.ggasoftware.indigo.IndigoObject;

public class Main {
    public static void main(String[] args) throws InterruptedException {
        MainFrame.setNativeLookAndFeel();
        MainFrame mf = new MainFrame();
        mf.setLocationRelativeTo(null);
        mf.setVisible(true);
    }
}
