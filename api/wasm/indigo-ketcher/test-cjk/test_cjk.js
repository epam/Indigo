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

    // Render
    {
        test("render", "CJK_characters", async () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "png");
            options.set("render-background-color","1,1,1");
            var fs = require('fs');
            const ket_data = fs.readFileSync("CJK_characters_test.ket");
            const png = Buffer.from(indigo.render(ket_data, options), "base64");
            fs.writeFileSync("CJK_characters_out.png", png);
            const {equal} = await looksSame('CJK_characters_ref.png', 'CJK_characters_out.png');
            assert(equal);
            options.delete();
        });

        test("render", "CJK_characters_2_styles_2_sizes", async () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "png");
            options.set("render-background-color","1,1,1");
            var fs = require('fs');
            const ket_data = fs.readFileSync("CJK_characters_2_styles_2_sizes_test.ket");
            const png = Buffer.from(indigo.render(ket_data, options), "base64");
            fs.writeFileSync("CJK_characters_2_styles_2_sizes_out.png", png);
            const {equal} = await looksSame('CJK_characters_2_styles_2_sizes_ref.png', 'CJK_characters_2_styles_2_sizes_out.png');
            assert(equal);
            options.delete();
        });

        test("render", "Characters_4_sets_4_styles", async () => {
            let options = new indigo.MapStringString();
            options.set("render-output-format", "png");
            options.set("render-background-color","1,1,1");
            var fs = require('fs');
            const ket_data = fs.readFileSync("Characters_4_sets_4_styles_test.ket");
            const png = Buffer.from(indigo.render(ket_data, options), "base64");
            fs.writeFileSync("Characters_4_sets_4_styles_out.png", png);
            const {equal} = await looksSame('Characters_4_sets_4_styles_ref.png', 'Characters_4_sets_4_styles_out.png');
            assert(equal);
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
