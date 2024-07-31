file(READ package.json packageJson)

string(REGEX REPLACE ".*\"version\": \"([^\"]*)\".*" "\\1" version ${packageJson})
message("Version is: ${version}")
# patch package.json
string(REPLACE "indigo-ketcher-separate-wasm.wasm" "indigo-ketcher-${version}.wasm" packageJson ${packageJson})
string(REPLACE "indigo-ketcher-separate-wasm-norender.wasm" "indigo-ketcher-norender-${version}.wasm" packageJson ${packageJson})

file(WRITE package.json "${packageJson}")

# patch indigo-ketcher-separate-wasm.js
file(READ indigo-ketcher-separate-wasm.js content)
string(REPLACE "indigo-ketcher-separate-wasm.wasm" "/indigo-ketcher-${version}.wasm" content "${content}")
string(REPLACE "fetch(binaryFile" "fetch(new URL(binaryFile, self.location.origin)" content "${content}")
file(WRITE "indigo-ketcher-separate-wasm.js" "${content}")

# patch indigo-ketcher-separate-wasm-norender.js
file(READ indigo-ketcher-separate-wasm-norender.js content)
string(REPLACE "indigo-ketcher-separate-wasm-norender.wasm" "/indigo-ketcher-norender-${version}.wasm" content "${content}")
string(REPLACE "fetch(binaryFile" "fetch(new URL(binaryFile, self.location.origin)" content "${content}")
file(WRITE "indigo-ketcher-separate-wasm-norender.js" "${content}")

# rename indigo-ketcher-separate-wasm.wasm
file(RENAME indigo-ketcher-separate-wasm.wasm indigo-ketcher-${version}.wasm)

# rename indigo-ketcher-separate-wasm-norender.wasm
file(RENAME indigo-ketcher-separate-wasm-norender.wasm indigo-ketcher-norender-${version}.wasm)
