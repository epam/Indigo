const moduleFn = require('./libindigo-ketcher.js')
const assert = require('assert').strict;

function testCalculate(indigo) {
    let options = new indigo.MapStringString();
    selected = new indigo.VectorInt();
    let molfile = `$RXN



  1  1  0
$MOL

  Ketcher  3262115472D 1   1.00000     0.00000     0

  5  4  0     0  0            999 V2000
    2.3000   -4.4330    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.3000   -4.4330    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.8000   -3.5670    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.9000   -6.9001    0.0000 S   0  0  0  0  0  0  0  0  0  0  0  0
    2.9000   -5.9001    0.0000 S   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  1  3  1  0     0  0
  4  5  1  0     0  0
M  END
$MOL

  Ketcher  3262115472D 1   1.00000     0.00000     0

  1  0  0     0  0            999 V2000
    6.3501   -4.1500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
M  END
`
    console.log(indigo.calculate(molfile, options, selected));
    assert.deepStrictEqual(
        JSON.parse(indigo.calculate(molfile, options, selected)),
        {
            "molecular-weight":"[42.0797410; 66.1458774] > [16.0424604]",
            "most-abundant-mass":"[42.0469501; 65.9597914] > [16.0313001]",
            "monoisotopic-mass":"[42.0469501; 65.9597914] > [16.0313001]",
            "mass-composition":"[C 85.63 H 14.37; H 3.05 S 96.95] > [C 74.87 H 25.13]",
            "gross-formula":"[C3 H6; H2 S2] > [C H4]"
        }
    );
    
    selected.push_back(5); // select CH4
    console.log(indigo.calculate(molfile, options, selected));
    assert.deepStrictEqual(
        JSON.parse(indigo.calculate(molfile, options, selected)),
        {"molecular-weight":"16.0424604","most-abundant-mass":"16.0313001","monoisotopic-mass":"16.0313001","mass-composition":"C 74.87 H 25.13","gross-formula":"C H4"}
    );

    selected.delete();
    selected = new indigo.VectorInt();

    selected.push_back(0); selected.push_back(1); selected.push_back(2); selected.push_back(3); selected.push_back(4); // select two-components reagent
    console.log(indigo.calculate(molfile, options, selected));
    assert.deepStrictEqual(
        JSON.parse(indigo.calculate(molfile, options, selected)),
        {"molecular-weight":"42.0797410; 66.1458774","most-abundant-mass":"42.0469501; 65.9597914","monoisotopic-mass":"42.0469501; 65.9597914","mass-composition":"C 85.63 H 14.37; H 3.05 S 96.95","gross-formula":"C3 H6; H2 S2"}
    );


    selected.delete();
    selected = new indigo.VectorInt();

    selected.push_back(0); selected.push_back(1); selected.push_back(2); selected.push_back(5); // select product and partial reagent
    console.log(indigo.calculate(molfile, options, selected));
    assert.deepStrictEqual(
        JSON.parse(indigo.calculate(molfile, options, selected)),
        {"molecular-weight":"[42.0797410] > [16.0424604]","most-abundant-mass":"[42.0469501] > [16.0313001]","monoisotopic-mass":"[42.0469501] > [16.0313001]","mass-composition":"[C 85.63 H 14.37] > [C 74.87 H 25.13]","gross-formula":"[C3 H6] > [C H4]"}
    );
    options.delete();
    selected.delete();
}


function testCheck(indigo) {
    let options = new indigo.MapStringString();
    let molfile = `
  SMMXDraw04061618152D

 27 26  2  0  0  0  0  0  0  0999 V2000
    6.5709   -4.1572    0.0000 N   0  3  0  0  0  4  0  0  0  0  0  0
    7.3070   -3.7322    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0432   -4.1572    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.7793   -3.7322    0.0000 Q   0  0  0  0  0  0  0  0  0  0  0  0
    9.5154   -4.1572    0.0000 A   0  0  0  0  0  0  0  0  0  0  0  0
   10.2515   -3.7322    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.9877   -4.1572    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.7238   -3.7322    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.4599   -4.1572    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.1960   -3.7322    0.0000 C   0  0  3  1  0  0  1  0  0  0  0  0
   13.9321   -4.1572    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.6683   -3.7322    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.4044   -4.1572    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.1405   -3.7322    0.0000 C   0  0  3  0  0  0  0  0  0  0  0  0
   16.8766   -4.1572    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   17.6128   -3.7322    0.0000 C   0  0  3  0  0  0  0  0  0  0  0  0
   18.3489   -4.1572    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   19.0850   -3.7322    0.0000 C   0  4  0  0  0  0  0  0  0  0  0  0
    9.5154   -5.0072    0.0000 C   0  0  3  0  0  0  0  0  0  0  0  0
    8.7793   -5.4322    0.0000 L   0  0  0  0  0  0  0  0  0  0  0  0
   10.2515   -5.4322    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.2515   -6.2822    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.9877   -6.7072    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.9877   -7.5572    0.0000 C   1  0  0  0  0  0  0  0  0  0  0  0
   11.7238   -7.9822    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.7238   -8.8322    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.4599   -9.2572    0.0000 L   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  5  6  1  0  0  0  0
  6  7  1  0  0  0  0
  7  8  1  0  0  0  0
  8  9  1  0  0  0  0
  9 10  1  0  0  0  0
 10 11  1  0  0  0  0
 11 12  1  0  0  0  0
 12 13  1  0  0  0  0
 13 14  1  0  0  0  0
 14 15  1  0  0  0  0
 15 16  1  0  0  0  0
 16 17  1  0  0  0  0
 17 18  1  0  0  0  0
  5 19  1  0  0  0  0
 19 20  1  0  0  0  0
 19 21  1  0  0  0  0
 21 22  1  0  0  0  0
 22 23  1  0  0  0  0
 23 24  1  0  0  0  0
 24 25  1  0  0  0  0
 25 26  1  0  0  0  0
 26 27  1  0  0  0  0
 20 F    5   6   7   8   9  17
 27 T    5   6   7   5  13  31
M  CHG  1   1   1
M  RAD  3  14   3  16   1  18   2
M  ISO  1  24  13
M  RBC  4   6  -1  21   4  23   3  25   2
M  SUB  1  12   2
M  UNS  1   8   1
M  ALS  20  7 F C   N   O   F   Cl  Br  I
M  ALS  27  5 T C   N   B   Al  Ga
M  END
`
    assert.deepStrictEqual(
        JSON.parse(indigo.check(molfile, "", options)),
        {
            "valence": "Structure contains query features, so valency could not be checked",
            "radicals": "Structure contains radicals,(13,15,17)",
            "stereo":"Structure contains stereocenters with undefined stereo configuration",
            "query": "Structure contains query features",
            "charge": "Structure has non-zero charge",
            "salt": "Not implemented yet: check salt",
            "ambigous_h": "Structure contains query features, so ambiguous H could not be checked"
        }
    );
    options.delete();
}

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
    selected.delete();
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
    selected.delete();
    selected = new indigo.VectorInt();
    try
    {
        console.log( indigo.calculate(rxn_smiles, options, selected) );
    } catch(e)
    {
        console.log( "Exception:" + e );
    }
    selected.delete();
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

    testCheck(indigo);
    
    testCalculate( indigo )
    
    options.delete();
})
