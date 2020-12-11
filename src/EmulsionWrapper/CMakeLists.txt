message (STATUS "EmulsionWrapper...")

add_executable(EmulsionWrapper
	EmulsionWrapper.cpp)

target_link_libraries(EmulsionWrapper
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
#	libEmulsionWrapper
)

# install the library in the lib folder
#install(TARGETS NTBM DESTINATION "${CMAKE_INSTALL_BINDIR}")
