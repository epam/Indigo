package com.ggasoftware.indigo.chemdiff;

public class CompareOptions {
   private boolean _arom_flag = true;
   private boolean _cistrans_ignore_flag = false;
   private boolean _stereocenters_ignore_flag = false;
   private boolean _is_changed = false;

   public CompareOptions(boolean arom_flag, boolean cistrans_ignore_flag,
                         boolean stereocenters_ignore_flag)
   {
      this._arom_flag = arom_flag;
      this._cistrans_ignore_flag = cistrans_ignore_flag;
      this._stereocenters_ignore_flag = stereocenters_ignore_flag;
   }

   public void setAromFlag( boolean arom_flag )
   {
      this._arom_flag = arom_flag;
      _is_changed = true;
   }

   public void setCisTransIgnoreFlag( boolean cistrans_ignore_flag )
   {
      this._cistrans_ignore_flag = cistrans_ignore_flag;
      _is_changed = true;
   }

   public void setStereocentersIgnoreFlag( boolean stereocenters_ignore_flag )
   {
      this._stereocenters_ignore_flag = stereocenters_ignore_flag;
      _is_changed = true;
   }

   public boolean getAromFlag()
   {
      return _arom_flag;
   }

   public boolean getCisTransIgnoreFlag()
   {
      return _cistrans_ignore_flag;
   }

   public boolean getStereocentersIgnoreFlag()
   {
      return _stereocenters_ignore_flag;
   }

   public void fix()
   {
      _is_changed = false;
   }

   public boolean isChanged()
   {
      return _is_changed;
   }
}
