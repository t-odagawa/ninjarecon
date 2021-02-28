message (STATUS "Test tools...")

add_executable(TestPosition
	TestPosition.cpp
	)

target_link_libraries(TestPosition
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

# install the execute in the bin folder
install(TARGETS TestPosition DESTINATION "${CMAKE_INSTALL_BINDIR}/test")