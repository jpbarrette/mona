cmake_minimum_required( VERSION 3.2 )

add_executable(mona main.c gl_utils.c mona.c stb_image.c)

target_link_libraries(mona ${OPENCL_LIBRARIES} )
target_link_libraries(mona ${OPENGL_LIBRARIES} )
target_link_libraries(mona ${GLEW_LIBRARIES} )
target_link_libraries(mona ${GLFW_LIBRARIES} )
       

configure_file(goal_fs.glsl $<TARGET_FILE_DIR:mona>/goal_fs.glsl COPYONLY)
configure_file(goal_vs.glsl $<TARGET_FILE_DIR:mona>/goal_vs.glsl COPYONLY)
configure_file(test_fs.glsl $<TARGET_FILE_DIR:mona>/test_fs.glsl COPYONLY)
configure_file(test_vs.glsl $<TARGET_FILE_DIR:mona>/test_vs.glsl COPYONLY)


# Do the copying of shader files.
set( SHADERS_FILES
     goal_fs.glsl
     goal_vs.glsl
     test_fs.glsl
     test_vs.glsl)
foreach( file_i ${SHADERS_FILES})
     add_custom_command(TARGET mona POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_SOURCE_DIR}/src/${file_i}
                       $<TARGET_FILE_DIR:mona>/)
endforeach( file_i )

# Copy data files
add_custom_command(TARGET mona POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/data
                       $<TARGET_FILE_DIR:mona>/data)


