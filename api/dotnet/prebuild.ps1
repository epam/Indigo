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

${resourceFolder} = [IO.Path]::GetFullPath("${PSScriptRoot}/Resource")
echo "Checking for native Indigo libraries in Resource folder ${resourceFolder}..."
${result} = 0
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Linux/x64/libindigo.so"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Linux/x86/libindigo.so"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Mac/10.7/libindigo.dylib"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x64/concrt140.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x64/indigo.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x64/msvcp140.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x64/vcruntime140.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x64/vcruntime140_1.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x86/concrt140.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x86/indigo.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x86/msvcp140.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x86/vcruntime140.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x86/vcruntime140_1.dll"))


if (!${result}) {
    throw "No native libraries found in resource folder '${resourceFolder}', only stubs created. Please add native libraries first"
}