message (STATUS "Emergency...")

add_executable(Emergency
	Emergency.cpp
#	TrackMatch.hpp
)

target_link_libraries(Emergency
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

# install the execute in the bin folder
install(TARGETS Emergency DESTINATION "${CMAKE_INSTALL_BINDIR}/Emergency")