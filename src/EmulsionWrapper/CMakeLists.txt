message (STATUS "EmulsionWrapper...")

add_executable(EmulsionWrapper
	EmulsionWrapper.cpp)

target_link_libraries(EmulsionWrapper
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY})
#	libHitConverter)
