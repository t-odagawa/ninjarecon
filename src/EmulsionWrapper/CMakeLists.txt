message (STATUS "EmulsionWrapper...")

add_executable(EmulsionWrapper
	EmulsionWrapper.cpp)

target_link_libraries(EmulsionWrapper
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

# install the execute in the bin folder
install(TARGETS EmulsionWrapper DESTINATION "${CMAKE_INSTALL_BINDIR}/EmulsionWrapper")
