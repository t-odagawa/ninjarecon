message (STATUS "FileSeparator...")

add_executable(FileSeparator
	FileSeparator.cpp)

target_link_libraries(FileSeparator
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
)
