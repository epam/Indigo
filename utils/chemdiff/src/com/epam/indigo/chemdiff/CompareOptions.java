package com.epam.indigo.chemdiff;

public class CompareOptions
{
   private boolean _arom_flag = true;
   private boolean _cistrans_ignore_flag = false;
   private boolean _stereocenters_ignore_flag = false;
   private boolean _unseparate_charges = true;

   public CompareOptions (boolean arom_flag, boolean cistrans_ignore_flag,
           boolean stereocenters_ignore_flag,
           boolean unseparate_charges)
   {
      this._arom_flag = arom_flag;
      this._cistrans_ignore_flag = cistrans_ignore_flag;
      this._stereocenters_ignore_flag = stereocenters_ignore_flag;
      this._unseparate_charges = unseparate_charges;
   }

   public void setAromFlag (boolean arom_flag)
   {
      this._arom_flag = arom_flag;
   }

   public void setCisTransIgnoreFlag (boolean cistrans_ignore_flag)
   {
      this._cistrans_ignore_flag = cistrans_ignore_flag;
   }

   public void setStereocentersIgnoreFlag (boolean stereocenters_ignore_flag)
   {
      this._stereocenters_ignore_flag = stereocenters_ignore_flag;
   }

   public void setUnseparateChargesFlag (boolean unseparate_charges)
   {
      this._unseparate_charges = unseparate_charges;
   }

   public boolean getAromFlag ()
   {
      return _arom_flag;
   }

   public boolean getCisTransIgnoreFlag ()
   {
      return _cistrans_ignore_flag;
   }

   public boolean getStereocentersIgnoreFlag ()
   {
      return _stereocenters_ignore_flag;
   }

   public boolean getUnseparateChargesFlag ()
   {
      return _unseparate_charges;
   }
}
