class IndigoObject
{
    /*
    * All Indigo entities should be subclasses of this root class.
    * This allows to mix them in collections and process by universal methods e.g.:
    * 
    * IndigoObject create(string text) {
     *   if (text is molecule file)
     *       return parseMolecule(text)
     *   if (text is reaction file)
     *       return parseReaction(text)
     *  etc.
     * If the both Molecule and Reaction are IndigoObjects, they can be returned here.
     * }
    * 
    */
};