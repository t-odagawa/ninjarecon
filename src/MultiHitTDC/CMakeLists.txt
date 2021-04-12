message (STATUS "MultiHitTDC...")

add_executable(MultiHitTDC
	MultiHitTDC.cpp
	MultiHitTDC.hpp)
add_executable(TotEval
	TotEval.cpp)

target_link_libraries(MultiHitTDC
	${ROOT_LIBRARIES}
	Spectrum
	${Geant4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)
target_link_libraries(TotEval
	${ROOT_LIBRARIES}
	${Geatn4_LIBRARIES}
	${Boost_LIBRARIES}
	${B2MC_LIBRARY}
	libNTBM
)

# install the execute in the bin folder
install(TARGETS MultiHitTDC DESTINATION "${CMAKE_INSTALL_BINDIR}/MultiHitTDC")
install(TARGETS TotEval DESTINATION "${CMAKE_INSTALL_BINDIR}/TotEval")