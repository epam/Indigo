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

${resourceFolder} = [IO.Path]::GetFullPath("${PSScriptRoot}/lib")
echo "Checking for native Indigo libraries in Resource folder ${resourceFolder}..."
${result} = 0
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Linux/x64/libindigo-inchi.so"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Linux/x86/libindigo-inchi.so"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Mac/10.7/libindigo-inchi.dylib"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x64/indigo-inchi.dll"))
touchIfNotExists([IO.Path]::GetFullPath("${resourceFolder}/Win/x86/indigo-inchi.dll"))


if (!${result}) {
    throw "No native libraries found in resource folder '${resourceFolder}', only stubs created. Please add native libraries first"
}
