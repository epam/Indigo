// post.js - appended to the end of the emscripten module
// Adds Module.renderAsync() for asynchronous SVG-to-PNG conversion

(function() {

    // Convert SVG string to PNG base64 using browser Canvas API
    function svgToPngBrowser(svgStr) {
        return new Promise(function(resolve, reject) {
            var img = new Image();
            img.onload = function() {
                try {
                    var canvas = document.createElement('canvas');
                    canvas.width = img.width;
                    canvas.height = img.height;
                    var ctx = canvas.getContext('2d');
                    ctx.drawImage(img, 0, 0);
                    var dataUrl = canvas.toDataURL('image/png');
                    resolve(dataUrl.split(',')[1]);
                } catch (e) {
                    reject(e);
                }
            };
            img.onerror = function() {
                reject(new Error('Failed to load SVG into Image for PNG conversion'));
            };
            img.src = 'data:image/svg+xml;charset=utf-8,' + encodeURIComponent(svgStr);
        });
    }

    // Convert SVG string to PNG base64 using sharp (Node.js)
    function svgToPngNode(svgStr) {
        var sharp = require('sharp');
        return sharp(Buffer.from(svgStr)).png().toBuffer().then(function(buf) {
            return buf.toString('base64');
        });
    }

    Module.renderAsync = async function(data, options) {
        var requestedFormat = 'svg';

        // Detect requested format and override to SVG for C++ engine
        if (options && typeof options.get === 'function') {
            try {
                var fmt = options.get('render-output-format');
                if (fmt) requestedFormat = fmt;
            } catch(e) {}

            if (requestedFormat === 'png') {
                options.set('render-output-format', 'svg');
            }
        }

        // Call synchronous C++ render — always returns base64-encoded SVG
        var base64Out = Module.render(data, options);

        // Restore original option so the caller's map isn't permanently mutated
        if (requestedFormat === 'png' && options && typeof options.set === 'function') {
            options.set('render-output-format', 'png');
        }

        if (requestedFormat !== 'png') {
            return base64Out;
        }

        // Decode base64 SVG to string
        var svgStr;
        if (typeof Buffer !== 'undefined') {
            svgStr = Buffer.from(base64Out, 'base64').toString('utf-8');
        } else {
            var binary = atob(base64Out);
            var bytes = new Uint8Array(binary.length);
            for (var i = 0; i < binary.length; i++) {
                bytes[i] = binary.charCodeAt(i);
            }
            svgStr = new TextDecoder().decode(bytes);
        }

        // Convert SVG to PNG using the appropriate environment
        if (typeof window !== 'undefined' && typeof document !== 'undefined') {
            return svgToPngBrowser(svgStr);
        } else if (typeof require !== 'undefined') {
            return svgToPngNode(svgStr);
        } else {
            throw new Error('No environment available for SVG to PNG conversion');
        }
    };
})();
