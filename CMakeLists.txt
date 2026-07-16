# BlupiMania 2 — SDL2 / OpenGL (ES) port
# Desktop (MinGW/MSVC test build) and Android (NDK) from the same tree.

cmake_minimum_required(VERSION 3.16)
project(blupimania2 CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PORT ${CMAKE_CURRENT_SOURCE_DIR}/port)

# game sources (list from projet1.vcxproj), minus the Windows/DirectX
# platform files that the port replaces
set(GAME_SOURCES
    array.cpp auto.cpp autocatapult.cpp autodock.cpp autodoor.cpp
    autofiole.cpp autoflash.cpp autogoal.cpp autojostle.cpp autolift.cpp
    automax1x.cpp automeca.cpp autoscrap.cpp blitz.cpp button.cpp camera.cpp
    cbottoken.cpp check.cpp cloud.cpp cmdtoken.cpp color.cpp compass.cpp
    control.cpp cryptfile.cpp d3dengine.cpp d3dmath.cpp displaytext.cpp
    edit.cpp editvalue.cpp event.cpp filebuffer.cpp gamerfile.cpp gauge.cpp
    group.cpp image.cpp iman.cpp interface.cpp key.cpp label.cpp light.cpp
    link.cpp list.cpp maindialog.cpp mainedit.cpp mainmap.cpp mainundo.cpp
    map.cpp math3d.cpp menu.cpp metafile.cpp misc.cpp model.cpp modfile.cpp
    motion.cpp motionbird.cpp motionblupi.cpp motionbot.cpp motionfish.cpp
    motiongun.cpp motionjet.cpp motionperfo.cpp motionptero.cpp
    motionsnake.cpp motionsubm.cpp motiontrax.cpp object.cpp particule.cpp
    planet.cpp profile.cpp progress.cpp pyro.cpp restext.cpp robotmain.cpp
    script.cpp scroll.cpp shortcut.cpp slider.cpp task.cpp taskadvance.cpp
    taskcatapult.cpp taskdive.cpp taskdock.cpp taskdrink.cpp taskfire.cpp
    taskgoal.cpp taskgoto.cpp taskgun.cpp tasklist.cpp taskmanager.cpp
    taskperfo.cpp taskpush.cpp taskroll.cpp tasktrax.cpp taskturn.cpp
    taskwait.cpp terrain.cpp text.cpp water.cpp window.cpp
)
list(TRANSFORM GAME_SOURCES PREPEND ${SRC}/)

# source list from CBot.dsp (CBotAddExpr/CBotCompExpr/ClassFILE are not built)
set(CBOT_SOURCES
    CBot.cpp CBotClass.cpp CBotFunction.cpp
    CBotIf.cpp CBotProgram.cpp CBotStack.cpp CBotString.cpp CBotToken.cpp
    CBotTwoOpExpr.cpp CBotVar.cpp CBotWhile.cpp
)
list(TRANSFORM CBOT_SOURCES PREPEND ${SRC}/CBot/)

set(PORT_SOURCES
    ${PORT}/compat/compat.cpp
    ${PORT}/compat/d3dutil.cpp
    ${PORT}/gl/glapi.cpp
    ${PORT}/gl/gldevice.cpp
    ${PORT}/sdl/sdlapp.cpp
    ${PORT}/sdl/sdltextr.cpp
    ${PORT}/sdl/sdlsound.cpp
    ${PORT}/sdl/sdljoystick.cpp
)

if(ANDROID)
    add_library(blupimania2 SHARED ${GAME_SOURCES} ${CBOT_SOURCES} ${PORT_SOURCES})
else()
    add_executable(blupimania2 ${GAME_SOURCES} ${CBOT_SOURCES} ${PORT_SOURCES})
endif()

# the compat shim headers must shadow <windows.h>, <d3d.h>, <ddraw.h> etc.
target_include_directories(blupimania2 BEFORE PRIVATE
    ${PORT}/compat
    ${PORT}/gl
    ${SRC}
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # -fms-compatibility: the 2001 codebase forward-declares enums (VC6 style)
    target_compile_options(blupimania2 PRIVATE
        -w
        -fno-strict-aliasing
        -fno-delete-null-pointer-checks
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(blupimania2 PRIVATE
        -fpermissive
        -w
        -fno-strict-aliasing
        # the 2001 codebase calls methods on NULL ("if (this == NULL) return;")
        -fno-delete-null-pointer-checks
    )
endif()

if(ANDROID)
    # SDL2 is built as a sibling project via the Android gradle build
    target_link_libraries(blupimania2 PRIVATE SDL2 GLESv2 log android)
else()
    find_package(SDL2 REQUIRED)
    target_include_directories(blupimania2 PRIVATE ${SDL2_INCLUDE_DIRS})
    if(WIN32)
        target_link_libraries(blupimania2 PRIVATE mingw32 SDL2::SDL2main SDL2::SDL2)
    else()
        target_link_libraries(blupimania2 PRIVATE SDL2::SDL2)
    endif()
endif()
