message (STATUS "FileSeparator...")

add_executable(FileSeparator
	FileSeparator.cpp)

target_link_libraries(FileSeparator
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
)

# install the execute in the bin folder
install(TARGETS FileSeparator DESTINATION "${CMAKE_INSTALL_BINDIR}/FileSeparator")
