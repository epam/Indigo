function copyOrTouchIfNotExists ([String]  ${srcLibRoot}, [String] ${libPath}, [String] ${destLibRoot}) {
    ${destPath} = ${libPath}.replace("darwin", "osx")
    ${destPath} = ${destPath}.replace("windows", "win")
    ${destPath} = ${destPath}.replace("x86_64/", "x64/native/")
    ${destPath} = ${destPath}.replace("i386/", "x86/native/")

    ${srcPath} = "${srcLibRoot}/${libPath}"
    ${destPath} = "${destLibRoot}/${destPath}"
    if (![System.IO.File]::Exists(${srcPath})) {
        echo "Not found ${srcPath}, creating stub for library ${destPath}"
        ${destPathParent} = [IO.Path]::GetFullPath("${destPath}/..")
        New-Item -ItemType Directory -Force -ErrorAction SilentlyContinue ${destPathParent}
        New-Item -ItemType File -Force -Path ${destPath}
    } else {
        echo "Copying native library from ${srcPath} to ${destPath}"
        ${destPathParent} = [IO.Path]::GetFullPath("${destPath}/..")
        New-Item -ItemType Directory -Force -ErrorAction SilentlyContinue ${destPathParent}
        Copy-Item ${srcPath} ${destPath} -Force
        ${global:result} += 1
    }
}

${distLibFolder} = [IO.Path]::GetFullPath("${PSScriptRoot}/../../../dist/lib")
${resourceFolder} = [IO.Path]::GetFullPath("${PSScriptRoot}/runtimes")

Remove-Item ${resourceFolder} -Recurse -Force -ErrorAction Ignore

copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "darwin-x86_64/libindigo.dylib" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "darwin-x86_64/libindigo-inchi.dylib" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "darwin-x86_64/libindigo-renderer.dylib" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "darwin-x86_64/libbingo-nosql.dylib" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "linux-x86_64/libindigo.so" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "linux-x86_64/libindigo-inchi.so" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "linux-x86_64/libindigo-renderer.so" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "linux-x86_64/libbingo-nosql.so" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "windows-x86_64/indigo.dll" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "windows-x86_64/indigo-inchi.dll" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "windows-x86_64/indigo-renderer.dll" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "windows-x86_64/bingo-nosql.dll" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "windows-x86_64/msvcp140.dll" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "windows-x86_64/vcruntime140.dll" -destLibRoot ${resourceFolder}
copyOrTouchIfNotExists -srcLibRoot ${distLibFolder} -libPath "windows-x86_64/vcruntime140_1.dll" -destLibRoot ${resourceFolder}

if (!${result}) {
    throw "No native libraries found in resource folder '${resourceFolder}', only stubs created. Please add native libraries first"
}
