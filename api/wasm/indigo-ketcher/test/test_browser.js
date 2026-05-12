const puppeteer = require('puppeteer');
const path = require('path');
const fs = require('fs');

(async () => {
    let browser;
    try {
        console.log("Starting browser test for renderAsync...");
        
        // Ensure indigo-ketcher.js is available (it should be copied to the current directory by CMake during tests)
        const scriptPath = path.join(__dirname, 'indigo-ketcher.js');
        if (!fs.existsSync(scriptPath)) {
            console.error("❌ Cannot find indigo-ketcher.js in " + __dirname);
            process.exit(1);
        }

        browser = await puppeteer.launch({ 
            headless: 'new',
            args: ['--no-sandbox', '--disable-setuid-sandbox', '--disable-web-security']
        });
        
        const page = await browser.newPage();
        
        // Pipe page console to node console
        page.on('console', msg => console.log('Browser log:', msg.text()));
        
        // We will expose a resolve/reject pair so the page context can communicate back to Node context
        const testPromise = new Promise(async (resolve, reject) => {
            await page.exposeFunction('reportResult', (success, message) => {
                if (success) {
                    console.log('✅ ' + message);
                    resolve();
                } else {
                    console.error('❌ ' + message);
                    reject(new Error(message));
                }
            });
        });

        // Add the WASM bundle script tag
        await page.addScriptTag({ path: scriptPath });

        // Run the test in the browser context
        await page.evaluate(async () => {
            try {
                // Determine the exported module factory name
                const moduleFactory = window.Module || window.indigoKetcher;
                if (typeof moduleFactory !== 'function') {
                    throw new Error("Could not find Module or indigoKetcher global factory function");
                }

                console.log("Instantiating WASM module...");
                const indigo = await moduleFactory();
                console.log("WASM module loaded successfully.");

                if (typeof indigo.renderAsync !== 'function') {
                    throw new Error("renderAsync method is missing on the instantiated module!");
                }

                // Render a simple molecule to PNG using the browser pipeline (Canvas)
                const options = new indigo.MapStringString();
                options.set("render-output-format", "png");
                
                const b64 = await indigo.renderAsync("C1=CC=CC=C1", options);
                options.delete();
                
                if (!b64 || typeof b64 !== 'string') {
                    throw new Error("renderAsync returned invalid output type: " + typeof b64);
                }
                
                // PNG magic bytes in base64 are "iVBORw0KGgo"
                if (!b64.startsWith("iVBORw0K")) {
                    throw new Error("PNG magic bytes validation failed. Output starts with: " + b64.substring(0, 30));
                }
                
                // Test SVG format via renderAsync just to ensure it works
                const optionsSvg = new indigo.MapStringString();
                optionsSvg.set("render-output-format", "svg");
                const svgOut = await indigo.renderAsync("C1=CC=CC=C1", optionsSvg);
                optionsSvg.delete();

                if (typeof window.atob === 'function') {
                    const decoded = window.atob(svgOut);
                    if (decoded.indexOf('<svg') === -1) {
                        throw new Error("SVG magic validation failed for SVG request");
                    }
                }
                
                window.reportResult(true, "Browser PNG render test passed!");
            } catch(e) {
                window.reportResult(false, e.toString());
            }
        });

        // Wait for the test to complete
        await testPromise;
        console.log("All browser tests passed.");
        process.exit(0);
    } catch (e) {
        console.error("Test framework error:", e);
        process.exit(1);
    } finally {
        if (browser) await browser.close();
    }
})();
