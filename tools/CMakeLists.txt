message (STATUS "Test tools...")

add_executable(TestPosition
	TestPosition.cpp
	)
add_executable(TestTangent
	TestTangent.cpp
	)
add_executable(TestBabyMindTangent
	TestBabyMindTangent.cpp
	)
add_executable(TestPositionDifference
	TestPositionDifference.cpp
	)
add_executable(TestOutput
	TestOutput.cpp
	)

target_link_libraries(TestPosition
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

target_link_libraries(TestTangent
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

target_link_libraries(TestBabyMindTangent
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

target_link_libraries(TestPositionDifference
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

target_link_libraries(TestOutput
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

# install the execute in the bin folder
install(TARGETS TestPosition TestTangent TestPositionDifference TestOutput
		DESTINATION "${CMAKE_INSTALL_BINDIR}/test"
		)