# doc CmakeLists.txt

find_package(Doxygen)

if (DOXYGEN_FOUND)
	configure_file(${WSIM_SOURCE_DIR}/doc/Doxyfile.cmake ${WSIM_BINARY_DIR}/doc/Doxyfile @ONLY)
	add_custom_target(doc ${DOXYGEN_EXECUTABLE})
endif (DOXYGEN_FOUND)

