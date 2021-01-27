# http://stackoverflow.com/questions/822404/how-to-set-up-cmake-to-build-an-app-for-the-iphone

EXEC_PROGRAM(xcodebuild ARGS -version OUTPUT_VARIABLE XCODE_VERSION)
string(REGEX MATCH "[0-9][.][0-9]" XCODE_VERSION ${XCODE_VERSION})

if(DEFINED ENV{UNIVERSAL} OR UNIVERSAL_BUILD)
    set(SSNAME "")
else()
    set(SSNAME ${SUBSYSTEM_NAME})
endif()

set(FRAMEWORK_PATH /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SSNAME}.sdk/System/Library/Frameworks)
set(FRAMEWORK_PATH /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks)
macro(ADD_FRAMEWORK fwname appname)
    set(FRAMEWORK_${fwname} ${FRAMEWORK_PATH}/${fwname}.framework)

    if(NOT IS_DIRECTORY ${FRAMEWORK_${fwname}})
        message(ERROR ": Framework ${fwname} not found")
    else()
        target_link_libraries(${appname} ${FRAMEWORK_${fwname}})
        message(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
endmacro()

macro(FIND_FRAMEWORK fwname)
    set(FRAMEWORK_${fwname} ${FRAMEWORK_PATH}/${fwname}.framework)

    if(NOT IS_DIRECTORY ${FRAMEWORK_${fwname}})
        message(ERROR ": Framework ${fwname} not found")
    else()
        message(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
endmacro()
