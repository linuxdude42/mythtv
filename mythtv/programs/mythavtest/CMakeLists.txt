add_executable(mythavtest mythavtest.cpp mythavtest_commandlineparser.cpp
                          mythavtest_commandlineparser.h)

target_include_directories(mythavtest PRIVATE .)

target_compile_definitions(
  mythavtest
  PRIVATE $<$<BOOL:${USING_OPENGL}>:USING_OPENGL>
          $<$<BOOL:${USING_VDPAU}>:USING_VDPAU>
          $<$<AND:$<BOOL:${USING_VAAPI}>,$<BOOL:${USING_OPENGL}>>:USING_VAAPI>
          $<$<BOOL:${USING_MMAL}>:USING_MMAL>
          $<$<BOOL:${MAC}>:USING_COREAUDIO>)

target_link_libraries(mythavtest PUBLIC myth mythtv mythbase mythui)

install(TARGETS mythavtest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
