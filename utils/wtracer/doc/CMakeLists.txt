# Copyright (C) 2005-2011 Antoine Fraboulet (http://wsim.gforge.inria.fr/)
#
# Use, modification and distribution is subject to WSIM's licensing terms
# See accompanying files LICENCE and AUTHORS for more details.

# doc CMakeLists.txt

FIND_PROGRAM(MAKEINFO_EXEC makeinfo)

IF (MAKEINFO_EXEC)
	SET(fig_in ${CMAKE_CURRENT_SOURCE_DIR}/fig)
	SET(fig_out ${CMAKE_CURRENT_BINARY_DIR}/wtracer/fig)
	SET(info_in ${CMAKE_CURRENT_SOURCE_DIR}/wtracer.texi)
	SET(info_out ${CMAKE_CURRENT_BINARY_DIR}/wtracer.info)
	SET(info_html ${CMAKE_CURRENT_BINARY_DIR}/index.html)

	ADD_CUSTOM_COMMAND(OUTPUT ${info_out}
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${fig_in} ${fig_out}
		COMMAND ${MAKEINFO_EXEC} -o ${info_out} ${info_in}
		DEPENDS ${info_in}
		COMMENT "Creating Info file ${info_out}"
		VERBATIM
	)

	ADD_CUSTOM_COMMAND(OUTPUT ${info_html}
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${fig_in} ${fig_out}
		COMMAND ${MAKEINFO_EXEC} --html ${info_in}
		DEPENDS ${info_in}
		COMMENT "Creating ${info_html} from texinfo"
		VERBATIM
	)
	
	ADD_CUSTOM_TARGET(wtracer-doc DEPENDS ${info_out})
	ADD_CUSTOM_TARGET(wtracer-html DEPENDS ${info_html})
ELSE ()
	MESSAGE("doc: MakeInfo is not found, can't make texinfo")
ENDIF (MAKEINFO_EXEC)



