# Copyright (C) 2005-2011 Antoine Fraboulet (http://wsim.gforge.inria.fr/)
#
# Use, modification and distribution is subject to WSIM's licensing terms
# See accompanying files LICENCE and AUTHORS for more details.

# doc CMakeLists.txt

FIND_PACKAGE(Doxygen)

# check if doxygen is even installed
if (DOXYGEN_FOUND)
	# prepare doxygen configuration file
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake ${WSIM_BINARY_DIR}/doc/Doxyfile @ONLY)
	# add doc as target
	ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE})
	
	# cleanup ${CMAKE_BINARY_DIR}/api-doc on "make clean"
	SET_PROPERTY(
		DIRECTORY APPEND PROPERTY
		ADDITIONAL_MAKE_CLEAN_FILES
		api-doc
	)

	SET(DOC_PATH "share/doc/${CPACK_PACKAGE_NAME}-${VERSION}")

	# install HTML API documentation and manual pages
	install(
		DIRECTORY   ${CMAKE_CURRENT_BINARY_DIR}/api-doc/html
		DESTINATION ${DOC_PATH}
	)

	# install man pages into packages, scope is now project root..
	INSTALL(
		DIRECTORY   ${CMAKE_CURRENT_BINARY_DIR}/api-doc/man/man3
		DESTINATION share/man/man3/
	)
endif (DOXYGEN_FOUND)

