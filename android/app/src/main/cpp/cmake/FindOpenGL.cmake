# FindOpenGL.cmake - Custom module for Android to provide OpenGL ES as OpenGL
# This tricks librw into thinking OpenGL was found, but uses GLES3 instead

if(ANDROID)
    # Find Android's OpenGL ES 3.0 and EGL libraries
    find_library(GLES3_LIBRARY GLESv3)
    find_library(EGL_LIBRARY EGL)
    
    if(GLES3_LIBRARY AND EGL_LIBRARY)
        set(OPENGL_FOUND TRUE)
        set(OpenGL_FOUND TRUE)
        
        # Create imported target OpenGL::EGL
        if(NOT TARGET OpenGL::EGL)
            add_library(OpenGL::EGL UNKNOWN IMPORTED)
            set_target_properties(OpenGL::EGL PROPERTIES
                IMPORTED_LOCATION "${EGL_LIBRARY}"
            )
        endif()
        
        # Create imported target OpenGL::OpenGL (pointing to GLES3)
        if(NOT TARGET OpenGL::OpenGL)
            add_library(OpenGL::OpenGL UNKNOWN IMPORTED)
            set_target_properties(OpenGL::OpenGL PROPERTIES
                IMPORTED_LOCATION "${GLES3_LIBRARY}"
            )
        endif()
        
        # Create imported target OpenGL::GL (pointing to GLES3)
        if(NOT TARGET OpenGL::GL)
            add_library(OpenGL::GL UNKNOWN IMPORTED)
            set_target_properties(OpenGL::GL PROPERTIES
                IMPORTED_LOCATION "${GLES3_LIBRARY}"
            )
        endif()
        
        message(STATUS "FindOpenGL (Android): Using GLESv3=${GLES3_LIBRARY}, EGL=${EGL_LIBRARY}")
    else()
        set(OPENGL_FOUND FALSE)
        set(OpenGL_FOUND FALSE)
        message(WARNING "FindOpenGL (Android): GLESv3 or EGL not found")
    endif()
else()
    # For non-Android platforms, use the standard FindOpenGL
    include(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
endif()
