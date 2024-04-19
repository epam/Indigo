file(READ package.json packageJson)

string(REGEX REPLACE ".*\"version\": \"([^\"]*)\".*" "\\1" version ${packageJson})
message("Version is: ${version}")
# patch package.json
string(REPLACE "indigo-ketcher.wasm" "indigo-ketcher-${version}.wasm" modifiedPackageJson ${packageJson})
file(WRITE package.json "${modifiedPackageJson}")

# patch indigo-ketcher.json
file(READ indigo-ketcher.js content)
string(REPLACE "indigo-ketcher.wasm" "indigo-ketcher-${version}.wasm" content "${content}")
file(WRITE "indigo-ketcher.js" "${content}")

# rename indigo-ketcher.wasm
file(RENAME indigo-ketcher.wasm indigo-ketcher-${version}.wasm)
