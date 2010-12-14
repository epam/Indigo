cd ..\

echo start javaw -jar -Xss10m legio.jar > dist\launch.bat

mkdir dist\lib
copy ..\api\java\dist\indigo-java.jar dist\lib\
copy ..\api\renderer\java\dist\indigo-renderer-java.jar dist\lib\

"%ProgramFiles%\NSIS\makensis.exe" installer\legio_installer.nsi