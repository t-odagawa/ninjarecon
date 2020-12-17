message (STATUS "HitConverter...")

add_executable(HitConverter
	HitConverter.cpp)

target_link_libraries(HitConverter
	${ROOT_LIBRARIES}
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
)
