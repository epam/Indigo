const indigoModuleFn = require('./indigo-ketcher.js')
const assert = require('assert').strict;
const looksSame = require('looks-same');

// Extremely simple test framework, thanks to @sohamkamari (https://github.com/sohamkamani/nodejs-test-without-library)
let tests = []

function test(group, name, fn) {
    tests.push({group, name, fn})
}

function parseHrtimeToSeconds(hrtime) {
    return (hrtime[0] + (hrtime[1] / 1e9)).toFixed(3);
}

function run() {
    let succeeded = 0;
    let failed = 0;
    console.log("Starting tests...\n")
    var startTestsTime = process.hrtime();
    tests.forEach(t => {
        try {
            var startTestTime = process.hrtime();
            t.fn()
            const elapsedSeconds = parseHrtimeToSeconds(process.hrtime(startTestTime));
            console.log(`✅ ${t.group}.${t.name} [${elapsedSeconds}s]`);
            succeeded++;
        } catch (e) {
            console.log(`❌ ${t.group}.${t.name}`)
            console.log(e.stack)
            failed++
        }
    })
    const elapsedSeconds = parseHrtimeToSeconds(process.hrtime(startTestsTime));
    const total = succeeded + failed;
    console.log(`\n${total} tests executed in ${elapsedSeconds} seconds. ${succeeded} succeeded, ${failed} failed.`)

    if (failed) {
        process.exit(1);
    }
}

// Tests definition
indigoModuleFn().then(indigo => {
    // Common constants
    const mol_smiles = "C1=CC=CC=C1";
    const mol_smiles_aromatized = "c1ccccc1";
    const qmol_smarts = "[$([NX1-]=[NX2+]=[NX1-]),$([NX1]#[NX2+]-[NX1-2])]";
    const rxn_smiles = "C1=CC=CC=C1.N>>C1=CC=CC=N1.[CH3-]";

    // Aromatize
    {
        test("aromatize", "basic", () => {
            var options = new indigo.MapStringString();
            const aromatized_smiles = indigo.convert(indigo.aromatize(mol_smiles, "molfile", options), "smiles", options);
            assert.equal(aromatized_smiles, mol_smiles_aromatized);
            options.delete();
        });
    }

    // Automap
    {
        test("automap", "basic", () => {
            let options = new indigo.MapStringString();
            assert.doesNotThrow(() => {
                const result = indigo.automap(rxn_smiles, "discard", "molfile", options);
                assert.equal(result.indexOf("$RXN"), 0);
            });
            options.delete();
        });
    }

    // Calculate
    {
        test("calculate", "basic", () => {
            let options = new indigo.MapStringString();
            selected = new indigo.VectorInt();
            const values = JSON.parse(indigo.calculate("C.N.P.O", options, selected));
            assert.equal(values['gross-formula'], "C H4; H3 N; H3 P; H2 O");
            selected.delete();
            options.delete();
        });

        test("calculate", "selected", () => {
            let options = new indigo.MapStringString();
            selected = new indigo.VectorInt();
            selected.push_back(1);
            selected.push_back(2);
            const values = JSON.parse(indigo.calculate("C.N.P.O", options, selected));
            assert.equal(values['gross-formula'], "H3 N; H3 P");
            selected.delete();
            options.delete();
        });

        test("calculate", "complex", () => {
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
            assert.deepStrictEqual(
                JSON.parse(indigo.calculate(molfile, options, selected)),
                {
                    "molecular-weight": "[42.0797410; 66.1458774] > [16.0424604]",
                    "most-abundant-mass": "[42.0469501; 65.9597914] > [16.0313001]",
                    "monoisotopic-mass": "[42.0469501; 65.9597914] > [16.0313001]",
                    "mass-composition": "[C 85.63 H 14.37; H 3.05 S 96.95] > [C 74.87 H 25.13]",
                    "gross-formula": "[C3 H6; H2 S2] > [C H4]"
                }
            );

            selected.push_back(5); // select CH4
            assert.deepStrictEqual(
                JSON.parse(indigo.calculate(molfile, options, selected)),
                {
                    "molecular-weight": "16.0424604",
                    "most-abundant-mass": "16.0313001",
                    "monoisotopic-mass": "16.0313001",
                    "mass-composition": "C 74.87 H 25.13",
                    "gross-formula": "C H4"
                }
            );

            selected.delete();
            selected = new indigo.VectorInt();

            selected.push_back(0);
            selected.push_back(1);
            selected.push_back(2);
            selected.push_back(3);
            selected.push_back(4); // select two-components reagent
            assert.deepStrictEqual(
                JSON.parse(indigo.calculate(molfile, options, selected)),
                {
                    "molecular-weight": "42.0797410; 66.1458774",
                    "most-abundant-mass": "42.0469501; 65.9597914",
                    "monoisotopic-mass": "42.0469501; 65.9597914",
                    "mass-composition": "C 85.63 H 14.37; H 3.05 S 96.95",
                    "gross-formula": "C3 H6; H2 S2"
                }
            );

            selected.delete();
            selected = new indigo.VectorInt();

            selected.push_back(0);
            selected.push_back(1);
            selected.push_back(2);
            selected.push_back(5); // select product and partial reagent
            assert.deepStrictEqual(
                JSON.parse(indigo.calculate(molfile, options, selected)),
                {
                    "molecular-weight": "[42.0797410] > [16.0424604]",
                    "most-abundant-mass": "[42.0469501] > [16.0313001]",
                    "monoisotopic-mass": "[42.0469501] > [16.0313001]",
                    "mass-composition": "[C 85.63 H 14.37] > [C 74.87 H 25.13]",
                    "gross-formula": "[C3 H6] > [C H4]"
                }
            );
            selected.delete();
            options.delete();
        })
    }

    // Check
    {
        test("check", "basic", () => {
            let options = new indigo.MapStringString();
            const values = JSON.parse(indigo.check(mol_smiles, "", options));
            assert.equal(values.coord, 'Structure has no atoms coordinates');
            options.delete();
        });

        test("check", "complex", () => {
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
                    "radicals": "Structure contains radicals: (13,15,17)",
                    "stereo": "Structure contains stereocenters with undefined stereo configuration",
                    "query": "Structure contains query features",
                    "charge": "Structure has non-zero charge",
                    "ambiguous_h": "Structure contains query features, so ambiguous H could not be checked"
                }
            );
            options.delete();
        })
    }

    // CIP
    {
        test("cip", "basic", () => {
            let options = new indigo.MapStringString();
            options.set("ignore-stereochemistry-errors", "true");
            const molfile_cip = indigo.calculateCip("CN1C=C(/C=C2/SC(=S)N(CC([O-])=O)C/2=O)C2=CC=CC=C12", "molfile", options);
            assert(molfile_cip.indexOf("INDIGO_CIP_DESC") !== -1);
            assert(molfile_cip.indexOf("(E)") !== -1);
            options.delete();
        });
    }

    // Clean2D
    {

        test("clean2d", "basic", () => {
            let options = new indigo.MapStringString();
            let selected = new indigo.VectorInt();
            assert.doesNotThrow(() => {
                indigo.clean2d(mol_smiles, "molfile", options, selected)
            });
            selected.delete();
            options.delete();
        });

        test("clean2d", "selected", () => {
            let options = new indigo.MapStringString();
            let selected = new indigo.VectorInt();
            selected.push_back(1);
            selected.push_back(2);
            assert.doesNotThrow(() => {
                indigo.clean2d(mol_smiles, "molfile", options, selected)
            });
            selected.delete();
            options.delete();
        });
    }

    // Convert
    {
        test("convert", "molfile2000", () => {
            let options = new indigo.MapStringString();
            options.set('molfile-saving-mode', '2000');
            const molfile_2000 = indigo.convert(mol_smiles, "molfile", options);
            assert(molfile_2000.indexOf("V3000") === -1);
            assert(molfile_2000.indexOf("V2000") !== -1);
            options.delete();
        });

        test("convert", "molfile3000", () => {
            let options = new indigo.MapStringString();
            options.set('molfile-saving-mode', '3000');
            const molfile_2000 = indigo.convert(mol_smiles, "molfile", options);
            assert(molfile_2000.indexOf("V3000") !== -1);
            assert(molfile_2000.indexOf("V2000") === -1);
            options.delete();
        });

        test("convert", "molfile3000_auto", () => {
            let options = new indigo.MapStringString();
            options.set('ignore-stereochemistry-errors', 'true');
            options.set('ignore-bad-valence', 'true');
            options.set('molfile-saving-mode', 'auto');
            const molfile_2000 = indigo.convert(`
  -INDIGO-12082220102D

  0  0  0  0  0  0  0  0  0  0  0 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 16 17 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 18.5771 -10.2084 0.0 0
M  V30 2 N 19.5168 -9.86641 0.0 0
M  V30 3 C 19.6905 -8.8816 0.0 0 CFG=2
M  V30 4 C 20.1905 -8.01558 0.0 0
M  V30 5 C 19.6905 -7.14955 0.0 0
M  V30 6 C 18.6905 -7.14955 0.0 0
M  V30 7 C 18.1905 -8.01558 0.0 0
M  V30 8 C 18.6905 -8.8816 0.0 0
M  V30 9 O 18.1905 -9.74763 0.0 0
M  V30 10 C 20.6302 -9.22362 0.0 0
M  V30 11 C 20.8038 -10.2084 0.0 0
M  V30 12 C 21.7435 -10.5504 0.0 0
M  V30 13 C 22.5095 -9.90766 0.0 0
M  V30 14 C 22.3359 -8.92285 0.0 0
M  V30 15 C 21.3962 -8.58083 0.0 0
M  V30 16 Cl 21.2225 -7.59603 0.0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 3 2 CFG=1
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 5 1 5 6
M  V30 6 1 6 7
M  V30 7 1 7 8
M  V30 8 1 8 3
M  V30 9 2 8 9
M  V30 10 1 3 10
M  V30 11 2 10 11
M  V30 12 1 11 12
M  V30 13 2 12 13
M  V30 14 1 13 14
M  V30 15 2 14 15
M  V30 16 1 15 10
M  V30 17 1 15 16
M  V30 END BOND
M  V30 BEGIN COLLECTION
M  V30 MDLV30/STERAC1 ATOMS=(1 3)
M  V30 END COLLECTION
M  V30 END CTAB
M  END
`, "molfile", options);
            assert(molfile_2000.indexOf("V3000") !== -1);
            assert(molfile_2000.indexOf("V2000") === -1);
            options.delete();
        });

        test("convert", "rxnfile2000", () => {
            let options = new indigo.MapStringString();
            options.set('molfile-saving-mode', '2000');
            const rxnfile_2000 = indigo.convert(rxn_smiles, "rxnfile", options);
            assert(rxnfile_2000.indexOf("V3000") === -1);
            assert(rxnfile_2000.indexOf("V2000") !== -1);
            options.delete();
        });

        test("convert", "rxnfile3000", () => {
            let options = new indigo.MapStringString();
            options.set('molfile-saving-mode', '3000');
            const rxnfile_3000 = indigo.convert(rxn_smiles, "rxnfile", options);
            assert(rxnfile_3000.indexOf("V3000") !== -1);
            assert(rxnfile_3000.indexOf("V2000") === -1);
            options.delete();
        });

        test("convert", "smiles", () => {
            let options = new indigo.MapStringString();
            const smiles = indigo.convert(mol_smiles, "smiles", options);
            assert.equal(smiles, "C1C=CC=CC=1");
            options.delete();
        });

        test("convert", "rsmiles", () => {
            let options = new indigo.MapStringString();
            const rsmiles = indigo.convert(rxn_smiles, "smiles", options);
            assert.equal(rsmiles, "C1C=CC=CC=1.N>>C1N=CC=CC=1.[CH3-]");
            options.delete();
        });

        test("convert", "smarts", () => {
            let options = new indigo.MapStringString();
            const smarts = indigo.convert(qmol_smarts, "smarts", options);
            assert.equal(smarts, "[$([NX1-]=[NX2+]=[NX1-]),$([NX1]#[NX2+]-[NX1-2])]");
            options.delete();
        });

        test("convert", "cml", () => {
            let options = new indigo.MapStringString();
            const cml = indigo.convert(mol_smiles, "cml", options);
            assert(cml.indexOf("<cml>") !== -1);
            assert(cml.indexOf("<molecule>") !== -1);
            options.delete();
        });

        test("convert", "rcml", () => {
            let options = new indigo.MapStringString();
            const rcml = indigo.convert(rxn_smiles, "cml", options);
            assert(rcml.indexOf("<cml>") !== -1);
            assert(rcml.indexOf("<reaction>") !== -1);
            options.delete();
        });

        test("convert", "inchi", () => {
            let options = new indigo.MapStringString();
            const inchi = indigo.convert(mol_smiles, "inchi", options);
            assert.equal(inchi, "InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H");
            options.delete();
        });

        test("convert", "inchi-key", () => {
            let options = new indigo.MapStringString();
            const inchi = indigo.convert(mol_smiles, "inchi-key", options);
            assert.equal(inchi, "UHOVQNZJYSORNB-UHFFFAOYSA-N");
            options.delete();
        });

        test("convert", "inchi-aux", () => {
            let options = new indigo.MapStringString();
            const inchi_aux = indigo.convert(mol_smiles, "inchi-aux", options);
            assert.equal(inchi_aux, "InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H\nAuxInfo=1/0/N:1,2,6,3,5,4/E:(1,2,3,4,5,6)/rA:6CCCCCC/rB:d1;s2;d3;s4;s1d5;/rC:;;;;;;");
            options.delete();
        });

        test("convert", "cdx_to_ket", () => {
            let options = new indigo.MapStringString();
            var fs = require('fs');
            const cdx_data = fs.readFileSync("test64.cdx");
            const ket = indigo.convert(cdx_data, "ket", options);
            const ket_data = indigo.convert(fs.readFileSync("test64.ket"), "ket", options);
            fs.writeFileSync("test64a.ket", ket);
            assert.equal(ket, ket_data);
            options.delete();
        });

        test("convert", "output-content-type", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            const smiles = indigo.convert(mol_smiles, "smiles", options);
            assert.equal(smiles, '{"struct":"C1C=CC=CC=1","format":"smiles","original_format":"chemical/x-daylight-smiles"}');
            options.delete();
        });

        test("convert", "input-format-smarts-short", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            options.set("input-format", "smarts");
            const smiles = indigo.convert(mol_smiles, "smiles", options);
            assert.equal(smiles, '{"struct":"C1C=CC=CC=1","format":"smiles","original_format":"chemical/x-daylight-smarts"}');
            options.delete();
        });

        test("convert", "input-format-smarts-long", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            options.set("input-format", "chemical/x-daylight-smarts");
            const smiles = indigo.convert(mol_smiles, "smiles", options);
            assert.equal(smiles, '{"struct":"C1C=CC=CC=1","format":"smiles","original_format":"chemical/x-daylight-smarts"}');
            options.delete();
        });

        test("convert", "rsmiles-app-json", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            const rsmiles = indigo.convert(rxn_smiles, "smiles", options);
            assert.equal(rsmiles, '{"struct":"C1C=CC=CC=1.N>>C1N=CC=CC=1.[CH3-]","format":"smiles","original_format":"chemical/x-daylight-smiles"}');
            options.delete();
        });

        test("convert", "rsmiles-input-format-smarts", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            options.set("input-format", "chemical/x-daylight-smarts");
            const rsmiles = indigo.convert(rxn_smiles, "smiles", options);
            assert.equal(rsmiles, '{"struct":"C1C=CC=CC=1.N>>C1N=CC=CC=1.[CH3-]","format":"smiles","original_format":"chemical/x-daylight-smarts"}');
            options.delete();
        });

    }

    // Convert explicit hydrogens
    {
        test("convert_explicit_hydrogens", "auto", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            const unfold_smiles = indigo.convert_explicit_hydrogens("CC", "auto", "smiles", options);
            assert.equal(unfold_smiles, '{"struct":"C([H])([H])([H])C([H])([H])[H]","format":"smiles","original_format":"chemical/x-daylight-smiles"}');
            const fold_smiles = indigo.convert_explicit_hydrogens("C([H])([H])([H])C([H])([H])[H]", "auto", "smiles", options);
            assert.equal(fold_smiles, '{"struct":"CC","format":"smiles","original_format":"chemical/x-daylight-smiles"}');
            options.delete();
        });

        test("convert_explicit_hydrogens", "fold", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            const fold_smiles = indigo.convert_explicit_hydrogens("C([H])([H])([H])C([H])([H])[H]", "fold", "smiles", options);
            assert.equal(fold_smiles, '{"struct":"CC","format":"smiles","original_format":"chemical/x-daylight-smiles"}');
            options.delete();
        });

        test("convert_explicit_hydrogens", "unfold", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            const unfold_smiles = indigo.convert_explicit_hydrogens("CC", "unfold", "smiles", options);
            assert.equal(unfold_smiles, '{"struct":"C([H])([H])([H])C([H])([H])[H]","format":"smiles","original_format":"chemical/x-daylight-smiles"}');
            options.delete();
        });

        test("convert_explicit_hydrogens", "auto_with_single_h", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            const unfold_smiles = indigo.convert_explicit_hydrogens("CC.[HH]", "auto", "smiles", options);
            assert.equal(unfold_smiles, '{"struct":"C([H])([H])([H])C([H])([H])[H].[H][H]","format":"smiles","original_format":"chemical/x-daylight-smiles"}');
            const fold_smiles = indigo.convert_explicit_hydrogens("C([H])([H])([H])C([H])([H])[H].[H][H]", "auto", "smiles", options);
            assert.equal(fold_smiles, '{"struct":"CC.[HH]","format":"smiles","original_format":"chemical/x-daylight-smiles"}');
            options.delete();
        });

        test("convert_explicit_hydrogens", "auto_single_h", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            const unfold_smiles = indigo.convert_explicit_hydrogens("[HH]", "auto", "smiles", options);
            assert.equal(unfold_smiles, '{"struct":"[H][H]","format":"smiles","original_format":"chemical/x-daylight-smiles"}');
            const fold_smiles = indigo.convert_explicit_hydrogens("[H][H]", "auto", "smiles", options);
            assert.equal(fold_smiles, '{"struct":"[HH]","format":"smiles","original_format":"chemical/x-daylight-smiles"}');
            options.delete();
        });

    }
    
    // Dearomatize
    {
        test("dearomatize", "basic", () => {
            let options = new indigo.MapStringString();
            const dearomatized_smiles = indigo.convert(indigo.dearomatize(mol_smiles_aromatized, "molfile", options), "smiles", options);
            assert.equal(dearomatized_smiles, "C1C=CC=CC=1");
            options.delete();
        });

        test("dearomatize", "query_mol", () => {
            let options = new indigo.MapStringString();
            const dearomatized_smiles = indigo.convert(indigo.dearomatize(mol_smiles_aromatized, "molfile", options), "smiles", options);
            assert.equal(dearomatized_smiles, "C1C=CC=CC=1");
            options.delete();
        });
    }

    // Layout
    {
        test("layout", "basic", () => {
            let options = new indigo.MapStringString();
            assert(indigo.layout(mol_smiles, "molfile", options).indexOf("-1.6") !== -1);
            options.delete();
        });
    }

    {
        test("layout", "smiles-ket", () => {
            const input_str = "CCC";
            let options = new indigo.MapStringString();
            options.set('aromatize-skip-superatoms', 'true');
            options.set('dearomatize-on-load', 'false');
            options.set('gross-formula-add-rsites', 'true');
            options.set('ignore-no-chiral-flag', 'false');
            options.set('ignore-stereochemistry-errors', 'true');
            options.set('input-format', "chemical/x-unknown");
            options.set('mass-skip-error-on-pseudoatoms', 'false');
            options.set('output-content-type', "application/json");
            options.set('smart-layout', 'true');
            let ket = indigo.layout(input_str, "ket", options);
            options.delete();
            assert(true);
        });
    }

    // Render
    {
        test("render", "svg", () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "svg");
            const svg = Buffer.from(indigo.render(mol_smiles, options), "base64").toString();
            assert(svg.indexOf("<svg") !== -1);
            options.delete();
        });

        test("render", "png", () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "png");
            const png = Buffer.from(indigo.render(mol_smiles, options), "base64");
            assert(png[0] === 137);
            assert(png[1] === 80);
            assert(png[2] === 78);
            assert(png[3] === 71);
            assert(png[4] === 13);
            assert(png[5] === 10);
            assert(png[6] === 26);
            assert(png[7] === 10);
            options.delete();
        });

        test("render", "pdf", () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "pdf");
            const pdf = Buffer.from(indigo.render(mol_smiles, options), "base64").toString();
            assert(pdf.indexOf("%PDF-") !== -1);
            options.delete();
        });

        // TODO: check if looksSame() works correctly for svg files
        // test("render", "utf8_svg", async () => {
        //     let options = new indigo.MapStringString();
        //     options.set("render-output-format", "svg");
        //     var fs = require('fs');
        //     const ket_data = fs.readFileSync("test_symbols_4_styles_2_sizes.ket");
        //     const svg = Buffer.from(indigo.render(ket_data, options), "base64");
        //     fs.writeFileSync("utf8_out.svg",svg);
        //     const {equal} = await looksSame('utf8_ref.svg', 'utf8_out.svg');
        //     assert(equal);
        //     options.delete();
        // });

        test("render", "utf8_png", async () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "png");
            options.set("render-background-color", "1,1,1");
            var fs = require('fs');
            const ket_data = fs.readFileSync("test_symbols_4_styles_2_sizes.ket");
            const png = Buffer.from(indigo.render(ket_data, options), "base64");
            fs.writeFileSync("utf8_out.png", png);
            const {equal} = await looksSame('utf8_ref.png', 'utf8_out.png');
            assert(equal);
            options.delete();
        });

        test("render", "ketcher_text_panel_regular", async () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "png");
            options.set("render-background-color", "1,1,1");
            var fs = require('fs');
            const ket_data = fs.readFileSync("ketcher_text_panel_test_regular.ket");
            const png = Buffer.from(indigo.render(ket_data, options), "base64");
            fs.writeFileSync("ketcher_text_panel_regular_out.png", png);
            const {equal} = await looksSame('ketcher_text_panel_regular_ref.png', 'ketcher_text_panel_regular_out.png');
            assert(equal);
            options.delete();
        });

        test("render", "ketcher_text_panel_bold", async () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "png");
            options.set("render-background-color", "1,1,1");
            var fs = require('fs');
            const ket_data = fs.readFileSync("ketcher_text_panel_test_bold.ket");
            const png = Buffer.from(indigo.render(ket_data, options), "base64");
            fs.writeFileSync("ketcher_text_panel_bold_out.png", png);
            const {equal} = await looksSame('ketcher_text_panel_bold_ref.png', 'ketcher_text_panel_bold_out.png');
            assert(equal);
            options.delete();
        });

        test("render", "ketcher_text_panel_italic", async () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "png");
            options.set("render-background-color", "1,1,1");
            var fs = require('fs');
            const ket_data = fs.readFileSync("ketcher_text_panel_test_italic.ket");
            const png = Buffer.from(indigo.render(ket_data, options), "base64");
            fs.writeFileSync("ketcher_text_panel_italic_out.png", png);
            const {equal} = await looksSame('ketcher_text_panel_italic_ref.png', 'ketcher_text_panel_italic_out.png');
            assert(equal);
            options.delete();
        });

        test("render", "ketcher_text_panel_bold_italic", async () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "png");
            options.set("render-background-color", "1,1,1");
            var fs = require('fs');
            const ket_data = fs.readFileSync("ketcher_text_panel_test_bold_italic.ket");
            const png = Buffer.from(indigo.render(ket_data, options), "base64");
            fs.writeFileSync("ketcher_text_panel_bold_italic_out.png", png);
            const {equal} = await looksSame('ketcher_text_panel_bold_italic_ref.png', 'ketcher_text_panel_bold_italic_out.png');
            assert(equal);
            options.delete();
        });
    }

    // Throws
    {
        test("throws", "wrong_compound", () => {
            let options = new indigo.MapStringString();
            assert.throws(() => {
                indigo.convert("C1C2", "molfile", options);
            });
            options.delete();
        });

        test("throws", "wrong_format", () => {
            let options = new indigo.MapStringString();
            assert.throws(() => {
                indigo.convert(mol_smiles, "smils", options);
            });
            options.delete();
        });

        test("throws", "wrong_options", () => {
            let options = new indigo.MapStringString();
            options.set("bool", "1");
            assert.throws(() => {
                indigo.convert(mol_smiles, "smiles", options);
            });
            options.delete();
        });

        test("throws", "wrong_selected", () => {
            let options = new indigo.MapStringString();
            const selected = [1, 2];
            assert.throws(() => {
                indigo.clean2d(mol_smiles, options, selected)
            });
            options.delete();
        });
    }

    // Version
    {
        test("version", "basic", () => {
            assert(indigo.version().indexOf("wasm") !== -1);
        });
    }

    // Version Info
    {
        test("versionInfo", "basic", () => {
            assert(indigo.versionInfo().indexOf("wasm") !== -1);
        });
    }

    // reactionComponents
    {
        test("reactionComponents", "basic", () => {
            let options = new indigo.MapStringString();
            assert.deepStrictEqual(JSON.parse(indigo.reactionComponents("C>O>N |$Carbon;Oxygen;Nitrogen$|", options)), {
                "reactants": ["C |$Carbon$|"],
                "catalysts": ["O |$Oxygen$|"],
                "products": ["N |$Nitrogen$|"]
            });
            options.delete();
        });
        test("reactionComponents", "complex_1", () => {
            let options = new indigo.MapStringString();
            assert.deepStrictEqual(JSON.parse(indigo.reactionComponents("[#6:1][C:2](=[O:3])[OH1:4].[C:5][N:6]>>[#6:1][C:2](=[O:3])[N:6][C:5] |$R1;;;OH;R2;NHR3;R1;;;NR3;R2$|", options)), {
                "reactants": ["CC([OH])=O |$R1;;OH;$|", "CN |$R2;NHR3$|"],
                "catalysts": [],
                "products": ["CC(NC)=O |$R1;;NR3;R2;$|"]
            });
            assert.deepStrictEqual(JSON.parse(indigo.reactionComponents("[#6:1][C:2](=[O:3])[OH1:4].[C:5][N:6]>>[#6:1][C:2](=[O:3])[N:6][C:5] |$R1;;;OH;R2;NHR3;R1;;;NR3;R2$|", options)), {
                "reactants": ["CC([OH])=O |$R1;;OH;$|", "CN |$R2;NHR3$|"],
                "catalysts": [],
                "products": ["CC(NC)=O |$R1;;NR3;R2;$|"]
            });
            options.delete();
        });
    }

    // RNA/DNA/PEPTIDE
    {
        test("PEPTIDE", "basic", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            options.set("input-format", "chemical/x-peptide-sequence");
            const peptide_seq_ref = "ACDEFGHIKLMNOPQRSRUVWY";
            const peptide_ket = indigo.convert(peptide_seq_ref, "ket", options);
            var fs = require('fs');
            // fs.writeFileSync("peptide_ref.ket", peptide_ket);
            const peptide_ket_ref = fs.readFileSync("peptide_ref.ket");
            assert.equal(peptide_ket, peptide_ket_ref.toString());

            const peptide_seq = indigo.convert(peptide_seq_ref, "sequence", options);
            // fs.writeFileSync("peptide_ref.seq", peptide_seq);
            const peptide_seq_ref1= fs.readFileSync("peptide_ref.seq");
            assert.equal(peptide_seq, peptide_seq_ref1.toString());
            options.delete();
        });
    }

    {
        test("RNA", "basic", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            options.set("input-format", "chemical/x-rna-sequence");
            const rna_seq_ref = "ACGTU";
            const rna_ket = indigo.convert(rna_seq_ref, "ket", options);
            var fs = require('fs');
            // fs.writeFileSync("rna_ref.ket", rna_ket);
            const rna_ket_ref = fs.readFileSync("rna_ref.ket");
            assert.equal(rna_ket, rna_ket_ref.toString());

            const rna_seq = indigo.convert(rna_seq_ref, "sequence", options);
            // fs.writeFileSync("rna_ref.seq", rna_seq);
            const rna_seq_ref1= fs.readFileSync("rna_ref.seq");
            assert.equal(rna_seq, rna_seq_ref1.toString());
            options.delete();
        });

    }

    {
        test("DNA", "basic", () => {
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            options.set("input-format", "chemical/x-dna-sequence");
            const dna_seq_ref = "ACGTU";
            const dna_ket = indigo.convert(dna_seq_ref, "ket", options);
            var fs = require('fs');
            // fs.writeFileSync("dna_ref.ket", dna_ket);
            const dna_ket_ref = fs.readFileSync("dna_ref.ket");
            assert.equal(dna_ket, dna_ket_ref.toString());

            const dna_seq = indigo.convert(dna_seq_ref, "sequence", options);
            // fs.writeFileSync("dna_ref.seq", dna_seq);
            const dna_seq_ref1= fs.readFileSync("dna_ref.seq");
            assert.equal(dna_seq, dna_seq_ref1.toString());
            options.delete();
        });

    }

    {
        test("PEPTIDE-FASTA", "basic", () => {
            var fs = require('fs');
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            options.set("input-format", "chemical/x-peptide-fasta");
            const fasta = fs.readFileSync("test_peptide.fasta");
            const peptide_ket = indigo.convert(fasta, "ket", options);
            const peptide_fasta = indigo.convert(fasta, "fasta", options);

            // fs.writeFileSync("test_peptide_ref.ket", peptide_ket);
            // fs.writeFileSync("test_peptide_ref.fasta", peptide_fasta);

            const peptide_ket_ref = fs.readFileSync("test_peptide_ref.ket");
            const peptide_fasta_ref = fs.readFileSync("test_peptide_ref.fasta");

            assert.equal(peptide_ket, peptide_ket_ref.toString());
            assert.equal(peptide_fasta, peptide_fasta_ref.toString());

            options.delete();
        });
    }

    {
        test("RNA-FASTA", "basic", () => {
            var fs = require('fs');
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            options.set("input-format", "chemical/x-rna-fasta");
            const fasta = fs.readFileSync("test_rna.fasta");
            const rna_ket = indigo.convert(fasta, "ket", options);
            const rna_fasta = indigo.convert(fasta, "fasta", options);

            // fs.writeFileSync("test_rna_ref.ket", rna_ket);
            // fs.writeFileSync("test_rna_ref.fasta", rna_fasta);

            const rna_ket_ref = fs.readFileSync("test_rna_ref.ket");
            const rna_fasta_ref = fs.readFileSync("test_rna_ref.fasta");

            assert.equal(rna_ket, rna_ket_ref.toString());
            assert.equal(rna_fasta, rna_fasta_ref.toString());

            options.delete();
        });
    }

    {
        test("DNA-FASTA", "basic", () => {
            var fs = require('fs');
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            options.set("input-format", "chemical/x-dna-fasta");
            const fasta = fs.readFileSync("test_dna.fasta");
            const dna_ket = indigo.convert(fasta, "ket", options);
            const dna_fasta = indigo.convert(fasta, "fasta", options);

            // fs.writeFileSync("test_dna_ref.ket", dna_ket);
            // fs.writeFileSync("test_dna_ref.fasta", dna_fasta);

            const dna_ket_ref = fs.readFileSync("test_dna_ref.ket");
            const dna_fasta_ref = fs.readFileSync("test_dna_ref.fasta");

            assert.equal(dna_ket, dna_ket_ref.toString());
            assert.equal(dna_fasta, dna_fasta_ref.toString());

            options.delete();
        });
    }

    {
        test("IDT", "basic", () => {
            var fs = require('fs');
            let options = new indigo.MapStringString();
            options.set("output-content-type", "application/json");
            options.set("input-format", "chemical/x-idt");
            const idt = "mA*mGC";
            const res_ket = indigo.convert(idt, "ket", options);
            const res_ket_ref = fs.readFileSync("idt_maxmgc.ket");
            assert.equal(res_ket, res_ket_ref.toString());
            let save_options = new indigo.MapStringString();
            save_options.set("output-content-type", "application/json");
            save_options.set("input-format", "chemical/x-indigo-ket");
            const res_idt = indigo.convert(res_ket, "idt", save_options);
            assert.equal(idt, res_idt);
            options.delete();
            save_options.delete();
        });
    }

    // Run tests
    run();
});
