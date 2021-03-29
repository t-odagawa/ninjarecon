message (STATUS "MultiHitTDC...")

add_executable(MultiHitTDC
	MultiHitTDC.cpp
	MultiHitTDC.hpp)

target_link_libraries(MultiHitTDC
	${ROOT_LIBRARIES}
	Spectrum
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

# install the execute in the bin folder
install(TARGETS MultiHitTDC DESTINATION "${CMAKE_INSTALL_BINDIR}/MultiHitTDC")