function touchIfNotExists {param(${checkedPath})
    if (![System.IO.File]::Exists(${checkedPath})) {
        echo "Creating stub for library ${checkedPath}"        
        New-Item -ItemType File -Force -Path ${checkedPath}
    } else {
        if ((Get-Item ${checkedPath}).Length -eq 0) {
            echo "Found stub for library ${checkedPath}"
        } else {
            echo "Found native library ${checkedPath}"
            ${global:result} += 1            
        }
        
    }
}

${resourceFolder} = [IO.Path]::GetFullPath("${PSScriptRoot}")
echo "Checking for native Indigo libraries in Resource folder ${resourceFolder}..."
${result} = 0
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/libindigo.so"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/libindigo.dylib"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/indigo.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/concrt140.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/msvcp140.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/vcruntime140.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/vcruntime140_1.dll"))
# Inchi
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/libindigo-inchi.so"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/libindigo-inchi.dylib"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/indigo-inchi.dll"))
# Renderer
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/libindigo-renderer.so"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/libindigo-renderer.dylib"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/indigo-renderer.dll"))
# Bingo
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/libbingo.so"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/libbingo.dylib"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/bingo.dll"))

if (!${result}) {
    throw "No native libraries found in resource folder '${resourceFolder}', only stubs created. Please add native libraries first"
}