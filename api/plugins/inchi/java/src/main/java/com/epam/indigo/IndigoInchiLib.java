/****************************************************************************
 * Copyright (C) 2011 EPAM Systems
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

package com.epam.indigo;

import com.sun.jna.Library;
import com.sun.jna.Pointer;

public interface IndigoInchiLib extends Library
{
    Pointer indigoInchiVersion ();
    int indigoInchiResetOptions ();
    int indigoInchiLoadMolecule (String inchi_string);
    Pointer indigoInchiGetInchi (int molecule);
    Pointer indigoInchiGetInchiKey (String inchi_string);
    Pointer indigoInchiGetWarning ();
    Pointer indigoInchiGetLog ();
    Pointer indigoInchiGetAuxInfo ();
}
