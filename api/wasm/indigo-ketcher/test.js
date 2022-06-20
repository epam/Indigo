const indigoModuleFn = require('./indigo-ketcher.js')
const assert = require('assert').strict;

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
                const result = indigo.automap(rxn_smiles, "discard", "molfile",options);
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
                indigo.clean2d(mol_smiles,"molfile",options, selected)
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
            const ket_data = indigo.convert(fs.readFileSync("test64.ket"),"ket", options);
            assert.equal(ket, ket_data);
            options.delete();
        });

    }

    // Dearomatize
    {
        test("dearomatize", "basic", () => {
            let options = new indigo.MapStringString();
            const dearomatized_smiles = indigo.convert(indigo.dearomatize(mol_smiles_aromatized, "molfile",  options), "smiles", options);
            assert.equal(dearomatized_smiles, "C1C=CC=CC=1");
            options.delete();
        });

        test("dearomatize", "query_mol", () => {
            let options = new indigo.MapStringString();
            assert.throws(() => {
                indigo.dearomatize(qmol_smarts, "molfile", options);
            });
            options.delete();
        });
    }

    // Layout
    {
        test("layout", "basic", () => {
            let options = new indigo.MapStringString();
            assert(indigo.layout(mol_smiles,"molfile", options).indexOf("-1.6") !== -1);
            options.delete();
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

    // Run tests
    run();
});
