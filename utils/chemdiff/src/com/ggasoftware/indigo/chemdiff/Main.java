package com.ggasoftware.indigo.chemdiff;

import java.util.ArrayList;
import java.util.List;

public class Main {

    public static void main(String[] args) {
      List<Integer> ints = new ArrayList<Integer>();
      List<? extends Number> nums = ints; // ok
      Integer q = 10;
      ints.add(q);


        MainFrame.setNativeLookAndFeel();
        MainFrame mf = new MainFrame();
        mf.setLocationRelativeTo(null);
        mf.setVisible(true);
    }
}
