cmake_minimum_required(VERSION 2.4.6)

project(mona)

find_package(PkgConfig)

pkg_check_modules(CAIRO cairo x11 cairo-xlib) # look into FindPkgConfig.cmake, 
                               # it contains documentation
# Now the variables CAIRO_INCLUDE_DIRS, CAIRO_LIBRARY_DIRS and CAIRO_LIBRARIES 
# contain what you expect 

link_directories(
    ${CAIRO_LIBRARY_DIRS} ${X11_LIBRARY_DIRS} ${CAIRO_XLIB_LIBRARY_DIRS} )

include_directories(
    ${CAIRO_INCLUDE_DIRS} ${X11_INCLUDE_DIRS} ${CAIRO_XLIB_INCLUDE_DIRS}  )

if (cairo_FOUND)
   add_definitions(-DSHOWWINDOW)
endif (cairo_FOUND)

add_executable(mona # name of the executable on Windows will be example.exe 
    mona.c )

target_link_libraries(mona 
    ${CAIRO_LIBRARIES} ${X11_LIBRARIES} ${CAIRO_XLIB_LIBRARIES} )
