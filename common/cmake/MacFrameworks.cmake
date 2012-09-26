# http://stackoverflow.com/questions/822404/how-to-set-up-cmake-to-build-an-app-for-the-iphone

EXEC_PROGRAM(xcodebuild ARGS -version OUTPUT_VARIABLE XCODE_VERSION)
STRING(REGEX MATCH "[0-9][.][0-9]" XCODE_VERSION ${XCODE_VERSION})
if (${XCODE_VERSION} GREATER 4.2)
    set(FRAMEWORK_PATH /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SUBSYSTEM_NAME}.sdk/System/Library/Frameworks)
else()
    set(FRAMEWORK_PATH /Developer/SDKs/MacOSX${SUBSYSTEM_NAME}.sdk/System/Library/Frameworks)
endif()


macro(ADD_FRAMEWORK fwname appname)
    set(FRAMEWORK_${fwname} ${FRAMEWORK_PATH}/${fwname}.framework)        

    if(NOT IS_DIRECTORY ${FRAMEWORK_${fwname}})
        MESSAGE(ERROR ": Framework ${fwname} not found")
    else()
        TARGET_LINK_LIBRARIES(${appname} ${FRAMEWORK_${fwname}})
        MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
endmacro()

macro(FIND_FRAMEWORK fwname)
    set(FRAMEWORK_${fwname} ${FRAMEWORK_PATH}/${fwname}.framework)        

    if(NOT IS_DIRECTORY ${FRAMEWORK_${fwname}})
        MESSAGE(ERROR ": Framework ${fwname} not found")
    else()
        MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
endmacro()