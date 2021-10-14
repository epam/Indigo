const moduleFn = require('./libindigo-ketcher.js')
const assert = require('assert').strict;

// Extremely simple test framework, thanks to @sohamkamari (https://github.com/sohamkamani/nodejs-test-without-library)
let tests = []
function test(name, fn) {
	tests.push({ name, fn })
}
function run() {
	tests.forEach(t => {
		try {
			t.fn()
			console.log('✅', t.name)
		} catch (e) {
			console.log('❌', t.name)
			console.log(e.stack)
		}
	})
}

moduleFn().then(indigo => {
    // Common constants and variables
    let options = new indigo.MapStringString();
    let mol_smiles = "C1=CC=CC=C1";
    let mol_smiles_aromatized = "c1ccccc1";
    let qmol_smarts = "[$([NX1-]=[NX2+]=[NX1-]),$([NX1]#[NX2+]-[NX1-2])]";
    let rxn_smiles = "C1=CC=CC=C1.N>>C1=CC=CC=N1.[CH3-]";

    // Define tests

    test("version", () => {
        assert(indigo.version().indexOf("wasm") != -1);
    });

    test("convert_molfile_options", () => {
        options.set('molfile-saving-mode', '3000');
        const molfile_3000 = indigo.convert(mol_smiles, "molfile", options);
        options.set('molfile-saving-mode', '2000');
        const molfile_2000 = indigo.convert(mol_smiles, "molfile", options);

        assert(molfile_2000.indexOf("V3000") == -1);
        assert(molfile_2000.indexOf("V2000") != -1);
        assert(molfile_3000.indexOf("V3000") != -1);
        assert(molfile_3000.indexOf("V2000") == -1);
    });

    test("convert_rxnfile_options", () => {
        options.set('molfile-saving-mode', '3000');
        const rxnfile_3000 = indigo.convert(rxn_smiles, "rxnfile", options);
        options.set('molfile-saving-mode', '2000');
        const rxnfile_2000 = indigo.convert(rxn_smiles, "rxnfile", options);

        assert(rxnfile_2000.indexOf("V3000") == -1);
        assert(rxnfile_2000.indexOf("V2000") != -1);
        assert(rxnfile_3000.indexOf("V3000") != -1);
        assert(rxnfile_3000.indexOf("V2000") == -1);
    });

    test("convert_smiles", () => {
        const smiles = indigo.convert(mol_smiles, "smiles", options);
        assert.equal(smiles, "C1C=CC=CC=1");
    });

    test("convert_rsmiles", () => {
        const rsmiles = indigo.convert(rxn_smiles, "smiles", options);
        assert.equal(rsmiles, "C1C=CC=CC=1.N>>C1N=CC=CC=1.[CH3-]");
    });

    test("convert_smarts", () => {
        const smarts = indigo.convert(qmol_smarts, "smarts", options);
        assert.equal(smarts, "[$([NX1-]=[NX2+]=[NX1-]),$([NX1]#[NX2+]-[NX1-2])]");
    });

    test("convert_cml", () => {
        const cml = indigo.convert(mol_smiles, "cml", options);
        assert(cml.indexOf("<cml>") != -1);
        assert(cml.indexOf("<molecule>") != -1);
    });

    test("convert_rcml", () => {
        const rcml = indigo.convert(rxn_smiles, "cml", options);
        assert(rcml.indexOf("<cml>") != -1);
        assert(rcml.indexOf("<reaction>") != -1);
    });

    test("convert_inchi", () => {
        const inchi = indigo.convert(mol_smiles, "inchi", options);
        console.log(inchi);
    });

    test("convert_inchi-aux", () => {
        const inchi_aux = indigo.convert(mol_smiles, "inchi_aux", options);
        console.log(inchi_aux);
    });

    test("convert_throws_wrong_compound", () => {
        assert.throws(() => {
            indigo.convert("C1C2", "molfile", options);
        });
    });

    test("convert_throws_wrong_format", () => {
        assert.throws(() => {
            indigo.convert(mol_smiles, "smils", options);
        });
    });

    test("convert_throws_wrong_options", () => {
        let wrong_options = new indigo.MapStringString();
        wrong_options.set("bool", "1");
        assert.throws(() => {
            indigo.convert(mol_smiles, "smiles", wrong_options);
        });
        delete wrong_options;
    });

    test("aromatize", () => {
        const aromatized_smiles = indigo.convert(indigo.aromatize(mol_smiles, options), "smiles", options);
        assert.equal(aromatized_smiles, mol_smiles_aromatized);
    });

    test("dearomatize", () => {
        const dearomatized_smiles = indigo.convert(indigo.dearomatize(mol_smiles_aromatized, options), "smiles", options);
        assert.equal(dearomatized_smiles, "C1C=CC=CC=1");
    });

    test("dearomatize_qmol", () => {
        assert.throws(() => {
            indigo.dearomatize(qmol_smarts, options);
        });
    });

    test("layout", () => {
        assert(indigo.layout(mol_smiles, options).indexOf("-1.6") != -1);
    });

    test("clean2d", () => {
        let selected = new indigo.VectorInt();
        assert.doesNotThrow(() => {
            indigo.clean2d(mol_smiles, options, selected)
        });
        delete selected;
    });

    test("clean2d_selected", () => {
        let selected = new indigo.VectorInt();
        selected.push_back(1);
        selected.push_back(2);
        assert.doesNotThrow(() => {
            indigo.clean2d(mol_smiles, options, selected)
        });
        delete selected;
    });

    test("calculate", () => {
        selected = new indigo.VectorInt();
        const values = JSON.parse(indigo.calculate("C.N.P.O", options, selected));
        assert.equal(values['gross-formula'], "C H4; H3 N; H3 P; H2 O");
        delete selected;
    });

    test("calculate_selected", () => {
        selected = new indigo.VectorInt();
        selected.push_back(1);
        selected.push_back(2);
        const values = JSON.parse(indigo.calculate("C.N.P.O", options, selected));
        assert.equal(values['gross-formula'], "H3 N; H3 P");
        delete selected;
    });

    test("automap", () => {
        assert.doesNotThrow(() => {
            const result = indigo.automap(rxn_smiles, "discard", options);
            assert.equal(result.indexOf("$RXN"), 0);
        });
    });

    test("check", () => {
        const values = JSON.parse(indigo.check(mol_smiles, "", options));
        assert.equal(values.coord, 'Structure has no atoms coordinates');
    });

    test("cip", () => {
        let cip_options = new indigo.MapStringString();
        cip_options.set("ignore-stereochemistry-errors", "true");
        const molfile_cip = indigo.calculateCip("CN1C=C(/C=C2/SC(=S)N(CC([O-])=O)C/2=O)C2=CC=CC=C12", cip_options);
        assert(molfile_cip.indexOf("INDIGO_CIP_DESC") != -1);
        assert(molfile_cip.indexOf("(E)") != -1);
        delete cip_options;
    });

    test("render_svg", () => {
        let render_options = new indigo.MapStringString();
        render_options.set("render-output-format", "svg");
        const svg = Buffer.from(indigo.render(mol_smiles, render_options), "base64").toString();
        assert(svg.indexOf("<svg") != -1);
        delete render_options;
    });

    test("render_png", () => {
        let render_options = new indigo.MapStringString();
        render_options.set("render-output-format", "png");
        const png = Buffer.from(indigo.render(mol_smiles, render_options), "base64");
        assert(png[0] == 137);
        assert(png[1] == 80);
        assert(png[2] == 78);
        assert(png[3] == 71);
        assert(png[4] == 13);
        assert(png[5] == 10);
        assert(png[6] == 26);
        assert(png[7] == 10);
        delete render_options;
    });

    test("render_pdf", () => {
        let render_options = new indigo.MapStringString();
        render_options.set("render-output-format", "pdf");
        const pdf = Buffer.from(indigo.render(mol_smiles, render_options), "base64").toString();
        assert(pdf.indexOf("%PDF-") != -1);
        delete render_options;
    });

    // Run tests
    run();

    // Clean up
    options.delete();
})
