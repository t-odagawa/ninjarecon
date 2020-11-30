message (STATUS "TrackMatch...")

add_executable(TrackMatch
	TrackMatch.cpp)

target_link_libraries(TrackMatch
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY})
#	libTrackMatch)
