message (STATUS "TrackMatch...")

add_executable(TrackMatch
	TrackMatch.cpp
	TrackMatch.hpp)

target_link_libraries(TrackMatch
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

# install the execute in the bin folder
install(TARGETS TrackMatch DESTINATION "${CMAKE_INSTALL_BINDIR}/TrackMatch")