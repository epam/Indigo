package com.ggasoftware.indigo.legio;

public class Main
{
    public static void main(String[] args) {
       MainFrame.setNativeLookAndFeel();
       MainFrame mf = new MainFrame();
       mf.setLocationRelativeTo(null);
       mf.setVisible(true);
    }
}
