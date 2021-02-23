const moduleFn = require('./libindigo-ketcher.js')

const promise = moduleFn({option1: 'test'})
promise.then(indigo => {
    // Check version
    console.log("********* Check version() *********")
    console.log(indigo.version());
    // Input data - map for options and input SMILES
    let options = new indigo.MapStringString();
    let mol_smiles = "C1=CC=CC=C1";
    let qmol_smarts = "[$([NX1-]=[NX2+]=[NX1-]),$([NX1]#[NX2+]-[NX1-2])]";
    let rxn_smiles = "C1=CC=CC=C1.N>>C1=CC=CC=N1.[CH3-]";
    // Check convert and options settings - first save molfile v2000, then v3000
    console.log("********* Check options settings - saving molfiles 2000 and 3000 *********");
    options.set('molfile-saving-mode', '3000');
    console.log(indigo.convert(mol_smiles, "molfile", options));
    console.log(indigo.convert(rxn_smiles, "rxnfile", options));
    options.set('molfile-saving-mode', '2000');
    console.log(indigo.convert(rxn_smiles, "rxnfile", options));
    console.log(indigo.convert(mol_smiles, "molfile", options));
    console.log(indigo.convert(mol_smiles, "smiles", options));
    console.log(indigo.convert(rxn_smiles, "smiles", options));
    console.log(indigo.convert(qmol_smarts, "smarts", options));
    console.log(indigo.convert(mol_smiles, "cml", options));
    console.log(indigo.convert(rxn_smiles, "cml", options));
    console.log(indigo.convert(mol_smiles, "inchi", options));
    console.log(indigo.convert(mol_smiles, "inchi-aux", options));
    // Check exception throwing for wrong input molecule
    console.log("********* Check exception throwing because of wrong input molecule *********")
    try {
        console.dir(indigo.convert("C1C2", "molfile", options));
    } catch (e) {
        console.log("Exception: " + e);
    }
    // Check exception throwing for wrong convert format
    console.log("********* Check exception throwing becaue of wrong convert format *********")
    try {
        console.dir(indigo.convert(mol_smiles, "smils", options));
    } catch (e) {
        console.log("Exception: " + e);
    }
    // Check aromatization and dearomatization
    console.log("********* Check aromatize *********")
    let aromatized_molecule = indigo.aromatize(mol_smiles, options)
    console.log("aromatized: " + indigo.convert(aromatized_molecule, "smiles", options));
    console.log("********* Check dearomatized *********")
    let dearomatized_molecule = indigo.dearomatize(aromatized_molecule, options)
    console.log("dearomatized: " + indigo.convert(dearomatized_molecule, "smiles", options));
    try {
        dearomatized_molecule = indigo.dearomatize(qmol_smarts, options);
    } catch (e) {
        console.log("Exception: " + e);
    }
    // Check layout
    console.log("********* Check layout *********")
    console.log(indigo.layout(mol_smiles, options));
    // Check clean2d
    console.log("********* Check clean2d *********")
    let selected = new indigo.VectorInt();
    console.log(indigo.clean2d(mol_smiles, options, selected));
    selected.push_back(1);
    selected.push_back(2);
    console.log(indigo.clean2d(mol_smiles, options, selected));
    // Check calculate
    console.log("********* Check calculate *********")
    selected = new indigo.VectorInt();
    try
    {
        console.log(indigo.calculate(`
  Ketcher  2102114592D 1   1.00000     0.00000     0

  2  1  0     0  0            999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.0000    0.0000    0.0000 X   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  END`, options, selected));
    } catch( e )
    {
        console.log( "Exception:" + e );
    }
    console.log(indigo.calculate("C.N.P.O", options, selected));
    selected.push_back(1);
    selected.push_back(2);
    console.log(indigo.calculate("C.N.P.O", options, selected));
    selected = new indigo.VectorInt();
    try
    {
        console.log( indigo.calculate(rxn_smiles, options, selected) );
    } catch(e)
    {
        console.log( "Exception:" + e );
    }
    // Check automap
    console.log("********* Check automap *********")
    console.log(indigo.convert(indigo.automap(rxn_smiles, "discard", options), "smiles", options));
    // Check check
    console.log("********* Check check *********")
    console.log(indigo.check(mol_smiles, "", options));
    // Check calculateCip
    console.log("********* Check calculateCip *********")
    options.set("ignore-stereochemistry-errors", "true")
    console.log(indigo.calculateCip("CN1C=C(/C=C2/SC(=S)N(CC([O-])=O)C/2=O)C2=CC=CC=C12", options));
    // Check render
    console.log("********* Check render *********")
    options.set("render-output-format", "svg")
    base64 = indigo.render("CC", options);
    console.log(base64);
    console.log(Buffer.from(base64, 'base64'));
    options.set("render-output-format", "png")
    base64 = indigo.render("CC", options);
    console.log(base64);
    console.log(Buffer.from(base64, 'base64'));
    options.set("render-output-format", "pdf")
    base64 = indigo.render("CC", options);
    console.log(base64);
    console.log(Buffer.from(base64, 'base64'));
})
