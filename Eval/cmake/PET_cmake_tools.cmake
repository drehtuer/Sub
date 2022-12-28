#######################################################
### Collection of useful cmake macros.              ###
### Used by:                                        ###
###	 PET_cmake_use_pkg                          ###
###                                                 ###
### This file includes macros for use in your cmake ###
### file and macros that are used by other macros   ###
### and shouldn't be called in your CMakeLists.txt  ###
###                                                 ###
### Overview of 'public' macros:                    ###
###	set_project_start( PROJECT_NAME                ###
###		(DEBUG|RELEASE)                        ###
###		[SHARED|STATIC] )                      ###
###	add_all_subdirectories( [directories] )        ###
###	create_executable( EXEC_NAME                   ###
###		[LINK lib1 ...]                             ###
###		[QTMOC header1 ...]                         ###
###		[QTUI ui1 ...]                              ###
###		[QTRCS rcs1 ...]                           ###
###		[USE_WINMAIN]                               ###
###		[RECURSIVE]                                 ###
###		[ADD_PATH path1 ...]                        ###
###		[FILES file1 ...]                           ###
###	create_library( LIB_NAME                            ###
###		[LINK lib1 ...]                             ###
###		[QTMOC header1 ...]                         ###
###		[QTUI ui1 ...]                              ###
###		[QTRCS rcs1 ...]                            ###
###		[RECURSIVE]                                 ###
###		[ADD_PATH path1 ...]                        ###
###		[FILES file1 ...]                           ###
###		[LINK_STATIC | LINK_SHARED]                 ###
###	create_test( TEST_NAME                              ###
###		[LINK lib1 ...]                             ###
###		[QTMOC header1 ...]                         ###
###		[QTUI ui1 ...]                              ###
###		[QTRCS rcs1 ...]                           ###
###		[ARGUMENTS arg1 ...]                        ###
###		[RECURSIVE]                                 ###
###		[ADD_PATH path1 ...]                        ###
###		[FILES file1 ...]                           ###
###		[USE_WINMAIN]                               ###
###	create_documentation( DOC_NAME                ###
###		[DOXYGEN_CONF conf.in]                ###
###	print_options( ${ARGN} )                    ###
###                                                 ###
### (A more detailed description can be found       ###
###  before the definition of each macro.)          ###
###                                                 ###
### Overview of 'private' macros:                   ###
###	For PET_cmake_use_pkg:                      ###
###		_check_pkg_args( [${ARGN}] )        ###
###		_check_pkg_found( PKG_NAME )        ###
###		_check_pkg_components( ${ARGN} )    ###
###		_check_pkg_version( PKG_NAME        ###
###			[${ARGN}] )                 ###
###		_check_pkg_includes( [${ARGN}] )    ###
###	For set_project_start():                    ###
###		_set_definitions()                  ###
###		_set_cmake_policies()               ###
###		_set_build_type()                   ###
###		_set_output_paths()                 ###
###		_set_linking_type( ${ARGN} )        ###
###		_set_windows_definitions()          ###
###		_set_unix_definitions()             ###
###	For create_executable and create_library:   ###
###		_add_all_files()                    ###
###		_glob_files                         ###
###		_check_create_args( ${ARGN} )       ###
###		_add_all_moc( ${{ARGN} )            ###
###		_add_all_ui( ${ARGN} )              ###
###		_add_all_rcs( ${ARGN} )             ###
###		_create_library_copy_dll( LIB_NAME )###
###		_copy_boost_dll( LIB_NAME )         ###
###		_create_target( ${ARGN} )           ###
###	                                            ###
#######################################################



### _check_pkg_args ###################################
### Used by PET_cmake_use_pkg to parse the input.   ###
###                                                 ###
### Currently used keyowrds:                        ###
###	COMPONENTS                                  ###
###	VERSION                                     ###
###	REQUIRED                                    ###
###	OPTIONAL                                    ###
### Parameter: List of arguments                    ###
### Uses: nothing                                   ###
### Sets:                                           ###
###	PKG_COMPONENTS                              ###
###	PKG_VERSION                                 ###
###	PKG_MSG_TYPE                                ###
#######################################################
macro( _check_pkg_args )

	set( PKG_COMPONENTS )
	set( PKG_VERSION )
	set( _PKG_ARG_VAR )
	set( PKG_MSG_TYPE
		"WARNING" )
	foreach( _PARAM ${ARGN} )
		string( TOUPPER
			${_PARAM}
			_UPARAM )
		
		if( _UPARAM STREQUAL "COMPONENTS" )
			set( _PKG_ARG_VAR
				"COMPONENTS" )
		else()
			if( _UPARAM STREQUAL "VERSION" )
				set( _PKG_ARG_VAR
					"VERSION" )
			else()
				if( _UPARAM STREQUAL "REQUIRED" )
					set( PKG_MSG_TYPE
						"FATAL_ERROR" )
				else()
					if( _UPARAM STREQUAL "OPTIONAL" )
						set( PKG_MSG_TYPE
							"STATUS" )
					else()
						if( PKG_ARG_VAR STREQUAL "" )
							message( FATAL_ERROR
								"Error while parsing arguments for use_pkg_*: No keyword as first argument." )
						else()
							set( PKG_${_PKG_ARG_VAR}
								${PKG_${_PKG_ARG_VAR}}
								${_PARAM} )
						endif()
					endif()
				endif()
			endif()
		endif()
	endforeach()
	set( _PARAM )
	set( _UPARAM )
	set( _PKG_ARG_VAR )
endmacro()



### _check_pkg_found ##################################
### Used by PET_cmake_use_pkg to check if a package ###
### was found. Depending if that package is         ###
### REQUIRED, OPTIONAL or not defined the script    ###
### will stop with an error, notify or warn if the  ###
### package wasn't found. There is no message if    ###
### the package was found. This message will be     ###
### generated by _check_pkg_version.                ###
###                                                 ###
### Parameter: _PKG_NAME (assumed to be matching    ###
###			 $(PKG_NAME)_FOUND (usually ###
###                      in upper case)             ###
### Uses: PKG_MSG_TYPE                              ###
### Sets: LAST_PKG_FOUND                            ###
#######################################################
macro( _check_pkg_found
	_PKG_NAME )

	set( LAST_PKG_FOUND
		true )
	if( NOT ${_PKG_NAME}_FOUND )
		message( ${PKG_MSG_TYPE}
			"\t" ${_PKG_NAME} " not found!" )
		set( LAST_PKG_FOUND
			false )
	endif()
	set( _PKG_NAME )
endmacro()



### _check_pkg_components #############################
### Used by PET_cmake_use_pkg to check if given     ###
### components were found.                          ###
###                                                 ###
### Parameter: List of components (assumed to be    ###
###			     matching ${ARG}_FOUND) ###
### Uses: PKG_MSG_TYPE                              ###
### Sets: nothing                                   ###
#######################################################
macro( _check_pkg_components )

	foreach( _COMPONENT ${ARGN} )
		if( NOT ${_COMPONENT}_FOUND )
			message( ${PKG_MSG_TYPE}
				"\t\t" ${_COMPONENT} " not found!" )
		else()
			message( STATUS
				"\t\t" ${_COMPONENT} " found." )
		endif()
	endforeach()
	set( _COMPONENT )
endmacro()



### _check_pkg_version ################################
### Used by PET_cmake_use_pkg to check the version  ###
### of a package. Can have 1 or 2 parameters:       ###
### 	1: No version is given, just say package    ###
###	   found.                                       ###
###	2: check if package version is greater or       ###
###	   equal to required version.                   ###
###                                                 ###
### Parameters:                                     ###
###	_PKG_NAME                                       ###
###	[package version]                               ###
### Uses:                                           ###
###	PKG_VERSION                                     ###
### Sets: nothing                                   ###	
#######################################################
macro( _check_pkg_version
	_PKG_NAME )

	if( ${ARGC} EQUAL 2 )
		if( NOT ${PKG_VERSION} VERSION_LESS  ${ARGN} AND
		    NOT ${PKG_VERSION} VERSION_EQUAL ${ARGN} )
			message( ${PKG_MSG_TYPE} 
				"\t" ${_PKG_NAME} " version: " ${ARGN} " < " ${PKG_VERSION} )
		else()
			message( STATUS
				"\t" ${_PKG_NAME} " version: " ${ARGN} " found." )
		endif()
	else()
		if( LAST_PKG_FOUND )
			message( STATUS
				"\t" ${_PKG_NAME} " found." )
		endif()
	endif()
	set( _PKG_NAME )
endmacro()



### _check_pkg_includes ###############################
### Used by PET_cmake_use_pkg to include            ###
### directories returned by find_package.           ###
###                                                 ###
### Parameters: List of include directories         ###
### Uses: LAST_PKG_FOUND                            ###
### Sets: nothing                                   ###
#######################################################
macro( _check_pkg_includes )

	if( LAST_PKG_FOUND )
		foreach( _INC ${ARGN} )
			include_directories( ${_INC} )
		endforeach()
	endif()
	set( _INC )
endmacro()



########################################################



### set_project_start #################################
### Use this macro after cmake_minimum_required and ###
### including all additional cmake scripts.         ###
### This macro sets all defintions and paths needed ###
### for a new project, if the variable              ###
### ${_PROJECT_NAME}_STANDALONE is set (e.g. by the ###
### bootstrap script with the cmake option -D).     ###
### If this variable is not set, it will skip all   ###
### definitions and assume to be part of a larger   ###
### project.                                        ###
###                                                 ###
### Parameters:                                     ###
###	_PROJECT_NAME                               ###
###	(DEBUG|RELEASE)                             ###
###	[SHARED|STATIC]                             ###
### Uses: ${_PROJECT_NAME}_STANDALONE               ###
### Sets: PROJECT_NAME (cmake)                      ###
#######################################################
macro( set_project_start
	_PROJECT_NAME
	_BUILD_TYPE )

	if( ${_PROJECT_NAME}_STANDALONE )
		project( ${_PROJECT_NAME} )
		message( STATUS
			"Standalone project: " ${PROJECT_NAME} )
		_set_definitions( ${_BUILD_TYPE} ${ARGN} )
	else()
		message( STATUS
			"\tSubproject: " ${_PROJECT_NAME} )
	endif()
	if( NOT LIBRARY_TYPE )
		message( FATAL_ERROR "LIBRARY_TYPE not defined. Did you forget to define this project as ${_PROJECT_NAME}_STANDALONE?" )
	endif()
	set( _PROJECT_NAME )
	set( _BUILD_TYPE )
endmacro()



### _set_definitions ##################################
### Wrapper for various other macros for project    ###
### definitions.                                    ###
###                                                 ###
### Parameters: (DEBUG|RELEASE) [SHARED|STATIC]     ###
### Uses: nothing                                   ###
### Sets: nothing                                   ###
#######################################################
macro( _set_definitions
	_BUILD_TYPE )
	
	_set_cmake_policies()
	_set_build_type( ${_BUILD_TYPE} )
	_set_windows_definitions()
	_set_unix_definitions()
	_set_output_paths()
	_set_linking_type( ${ARGN} )
	set( _BUILD_TYPE )
endmacro()



### _set_cmake_policies ###############################
### Sets CMake policies.                            ###
###                                                 ###
### Parameters: none                                ###
### Uses:                                           ###
###	CMAKE_MAJOR_VERSION (cmake)                 ###
###	CMAKE_MINOR_VERSION (cmake)                 ###
###	CMAKE_PATCH_VERSION (cmake)                 ###
### Sets: nothing                                   ###
### Policies:                                       ###
### CMP0001 NEW : no cmake backwards compatibility  ###
### CMP0002 NEW : target names must be unique       ###
### CMP0003 NEW : libraries linked with full path   ###
###               no longer produce linker search   ###
###               paths                             ###
### CMP0004 NEW : no leading or trailing white      ###
###               spaces for libraries              ###
### CMP0005 NEW : preprocessor definitions escaped  ###
###               automatically                     ###
### CMP0008 NEW : libraries with full path must be  ###
###               valid files                       ###
### CMP0010 NEW : bad variable syntax is an error   ###
### CMP0013 NEW : disallow duplicated binary dirs   ###
### CMP0015 OLD : link_directories not relativ to   ###
###               current dir                       ###
#######################################################
macro( _set_cmake_policies )

	if( ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.6.0 )
		cmake_policy( SET CMP0001 NEW )
		cmake_policy( SET CMP0002 NEW )
		cmake_policy( SET CMP0003 NEW )
		cmake_policy( SET CMP0004 NEW )
		cmake_policy( SET CMP0005 NEW )
	endif()
	if( ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.6.1 )
		cmake_policy( SET CMP0008 NEW )
	endif()
	if( ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.6.3 )
		cmake_policy( SET CMP0010 NEW )
	endif()
	if( ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.8.0 )
		cmake_policy( SET CMP0013 NEW )
	endif()
	if( ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.8.1 )
		cmake_policy( SET CMP0015 OLD )
	endif()
endmacro()



### _set_build_type ###################################
### Sets build type (Debug|Release).                ###
###                                                 ###
### Parameters: (DEBUG|RELEASE)                     ###
### Uses: APPLE (cmake)                             ###
### Sets:                                           ###
###	CMAKE_BUILD_TYPE                                ###
###	[CMAKE_CONFIGURATION_TYPES] (on APPLE)          ###
#######################################################
macro( _set_build_type
	_BUILD_TYPE )

	set( CMAKE_BUILD_TYPE
		${_BUILD_TYPE} )

	if( _BUILD_TYPE STREQUAL "RELEASE" AND APPLE )
		# Set XCode GUI to Release as default.
	    set( CMAKE_CONFIGURATION_TYPES
			Release )
	endif()
	set( _BUILD_TYPE )
endmacro()



### _set_output_paths #################################
### Sets output paths for executables and libraries ###
###                                                 ###
### Parameters: none                                ###
### Uses:                                           ###
###	PROJECT_NAME (cmake)                        ###
###	CMAKE_SYSTEM_NAME (cmake)                   ###
### Sets:                                           ###
###	EXECUTABLE_OUTPUT_PATH                      ###
###	LIBRARY_OUTPUT_PATH                         ###
###	DOC_OUTPUT_PATH                             ###
#######################################################
macro( _set_output_paths )

	set( EXECUTABLE_OUTPUT_PATH
		${${PROJECT_NAME}_SOURCE_DIR}/bin )
	set( LIBRARY_OUTPUT_PATH
		${${PROJECT_NAME}_SOURCE_DIR}/lib/${CMAKE_SYSTEM_NAME} )
	set( DOC_OUTPUT_PATH
		${${PROJECT_NAME}_SOURCE_DIR}/doc/doxygen )
endmacro()



### _set_linking_type #################################
### Sets default linking type (SHARED|STATIC).      ###
### If no parameter is given, it will set linking   ###
### on Unix or Apple systems to dynamic and on      ###
### Windows systems to static.                      ###
###                                                 ###
### Parameters: [SHARED|STATIC]                     ###
### Uses:                                           ###
###	UNIX (cmake)                                ###
###	APPLE (cmake)                               ###
### Sets: LIBRARY_TYPE                              ###
#######################################################
macro( _set_linking_type )
	if( NOT ${ARGN} STREQUAL "" )
		set( LIBRARY_TYPE
			${ARGN} )
	else()
		if( UNIX OR APPLE )
			set( LIBRARY_TYPE
				SHARED )
		else()
			set( LIBRARY_TYPE
				STATIC )
		endif()
	endif()
	message( STATUS
		"Linking type: " ${LIBRARY_TYPE} )
endmacro()



### _set_windows_defintions ###########################
### Sets definitions for Windows compilers like     ###
### Visual Studio.                                  ###
###                                                 ###
### Parameters: none                                ###
### Uses:                                           ###
###	WIN32 (cmake)                                   ###
###	CMAKE_CL_64 (cmake)                             ###
### Sets: nothing                                   ###
### Compiler flags:                                 ###
###	 /W3 /wd4996 /wd4290 /MP /MDd [/D WIN64]        ###
#######################################################
macro( _set_windows_definitions )

	if( WIN32 )
		# Visual Studio

		# Warning level 3
		add_definitions( /W3 )

		# Disable warnings for deprecated functions
		add_definitions( /wd4996 )

		# Disable warnings for throw
		add_definitions( /wd4290 )

		# Multiprocess compiling
		add_definitions( /MP )

		# Multithread- and DLL-specific versions of
		# the run-time routines are selected from
		# the standard .h files.
		# Defines _DEBUG, _MT, _DLL
		#add_definitions( /MDd )      
	endif()
	if( CMAKE_CL_64 )
		# Tell OpenCV that we are WIN64
		add_definitions( /D WIN64 ) # Still needed in OpenCV 2.1 ?
	endif()
endmacro()



### _set_unix_definitions #############################
### Sets compiler flags for Unix systems (gcc).     ###
###                                                 ###
### Parameters: none                                ###
### Uses:                                           ###
###	CMAKE_COMPILER_IS_GNUCC (cmake)                 ###
###	CMAKE_CXX_FLAGS (cmake)                         ###
### Sets: CMAKE_CXX_FLAGS                           ###
### Compiler flags:                                 ###
###	${CMAKE_CXX_FLAGS} -Wall -Wextra                ###
#######################################################
macro( _set_unix_definitions )

	if( CMAKE_COMPILER_IS_GNUCC )
        	set ( CMAKE_CXX_FLAGS
			"${CMAKE_CXX_FLAGS} -Wall -Wextra" )
	endif()
endmacro()



### optimize ###########################################
### Set compiler optimization                        ###
### Parameters: sse2, sse3, ssse3, sse4.1, sse4.2,   ###
### sse4a, avx                                       ###
### Uses:                                            ###
### Sets:                                            ###
### Unsets:                                          ###
########################################################
macro( optimizeFlags )
	set( _OPTIMIZATION_FLAGS )
	if( WIN32 )
		# msvc
		foreach( _FLAG ${ARGN} )
			if( _FLAG STREQUAL "sse2" )
				set( _OPTIMIZATION_FLAGS
					${_OPTIMIZATION_FLAGS}
					"/arch:SSE2" )
			else()
				if( _FLAG STREQUAL "sse" )
					set( _OPTIMIZATION_FLAGS
						${_OPTIMIZATION_FLAGS}
						"/arch:SSE" )
				endif()
			endif()
		endforeach()
		add_definitions( ${_OPTIMIZATION_FLAGS} )
	else()
		# gcc
		set( _OPTIMIZATION_FLAGS
			${_OPTIMIZATION_FLAGS}
			"-mtune=native" )
		foreach( _FLAG ${ARGN} )
			set( _OPTIMIZATION_FLAGS
				${_OPTIMIZATION_FLAGS}
				"-m${_FLAG}" )
			if( _FLAG STREQUAL "sse" OR _FLAG STREQUAL "sse2" )
				set( _OPTIMIZATION_FLAGS
					${_OPTIMIZATION_FLAGS}
					"-mfpmath=sse" )
			endif()
		endforeach()
		set( CMAKE_CXX_FLAGS
			${CMAKE_CXX_FLAGS}
			${_OPTIMIZATION_FLAGS} )
		# TODO altivec support?
	endif()

	message( STATUS "Using optimization flags: " ${_OPTIMIZATION_FLAGS} )

	set( _OPTIMIZATION_FLAGS )
	set( _FLAG )

endmacro()


#################################################################



### add_all_subdirectories ############################
### add_subdirectory() for all subdirectories.      ###
### Searches all directories in current directory   ###
### for CMakeLists.txt. It also looks for a         ###
### corresponding include directory and, if one     ###
### does exist, adds it to the include_directories. ###
### It is safe to call this macro in subprojects,   ###
### since ${PROJECT_NAME}_${SUB_RELATIV_DIR} is     ###
### always unique.                                  ###
###                                                 ###
### Parameters: [directories]                       ###
### Uses: ${_DIR}                                   ###
### Sets:                                           ###
###	SUB_RELATIV_DIR                                 ###
###	${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH     ###
###	${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH     ###
#######################################################
macro( add_all_subdirectories )

	set( _SEARCHDIRS )
	if( ${ARGC} GREATER 0 )
		foreach( _DIR ${ARGN} )
			list( APPEND _SEARCHDIRS
				${_DIR} )
		endforeach()
	else()
		set( _SEARCHDIRS
			* )
	endif()
	# get all files in current directory (absolut path)
	file( GLOB _SUBDIRS
		${_SEARCHDIRS} )
	foreach( _DIR ${_SUBDIRS} )
		# test if directory
		if( IS_DIRECTORY ${_DIR} )
			set( DIR_TABS_TOP
				${DIR_TABS} )
			set( DIR_TABS
				${DIR_TABS}\t )
			# get relativ path of directory
			string( REPLACE
				${CMAKE_CURRENT_SOURCE_DIR}/ ""
				SUB_RELATIV_DIR
				${_DIR} )
			# Does directory contain CMakeLists.txt?
			if( EXISTS ${_DIR}/CMakeLists.txt )
				message( STATUS ${DIR_TABS} "/" ${SUB_RELATIV_DIR} )
				# check if a include/${SUB_RELATIV_DIR} exists. If it does, set include path there,
				# else set include path to src/${SUB_RELATIV_DIR}


				# do we have include/ and src/ ?
				if( EXISTS ${_DIR}/include AND IS_DIRECTORY ${_DIR}/include )
					set( ${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH
						${_DIR}/include )
				endif()
				if( EXISTS ${_DIR}/src AND IS_DIRECTORY ${_DIR}/src )
					set( ${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH
						${_DIR}/src )
				endif()

				# if not already found, do more guessing on inc and src dirs
				if( NOT ${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH )
					# assuming the split occured before at an earlier src - include
					# and there is only one src or include in every path string
					string( REPLACE
						"/src" "/include"
						SUB_INCLUDE_DIR
						${_DIR} )
					if( EXISTS ${SUB_INCLUDE_DIR} AND IS_DIRECTORY ${SUB_INCLUDE_DIR} )
						set( ${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH 
							${SUB_INCLUDE_DIR} )
					else()
						if( ${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH )
							set( ${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH
								${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH )
						else()
							set( ${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH
								${_DIR} )
						endif()
					endif()
				endif()
				if( NOT ${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH )
					set( ${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH
						${_DIR} ) # let's assume CMakeLists.txt is in the same dir as the source files
				endif()

				# process ${_DIR}/CMakeLists.txt
				add_subdirectory( ${SUB_RELATIV_DIR} )
			endif()
			set( DIR_TABS
				${DIR_TABS_TOP} )
		endif()
	endforeach()
	set( _DIR )
	set( _SEARCHDIRS )
endmacro()



### _add_all_files ####################################
### Adds all files (*.h, *.hpp, *.c, *.cc, *.cpp) in###
### ${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH and ###
### ${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH to  ###
### ${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES.       ###
### If parameter SUB_ADD_INCLUDES is set, it will   ###
### add the include directory to global variable    ###
### include_directories.                            ###
###                                                 ###
### Parameters: none                                ###
### Uses:                                           ###
### ${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH     ###
### ${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH     ###
### GLOB_RECURSIVE                                  ###
### SUB_ADD_PATHS                                   ###
### Sets:                                           ###
### ${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES        ###
#######################################################
macro( _add_all_files )
	if( CREATE_FILES )
		set( ${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES )
		foreach( _FILE ${CREATE_FILES} )
			list( APPEND ${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES
				${_FILE} )
		endforeach()

		set( _FILE )
	else()
		set( _GLOB_STYLE
			GLOB )
		if( GLOB_RECURSIVE )
			set( _GLOB_STYLE
				GLOB_RECURSE )
			message( STATUS ${DIR_TABS} "\t(Adding subdirectories recursively)" )
		endif()

		set( _ADD_PATHS
			${${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH}
			${${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH} )
		foreach( _PATH ${CREATE_ADD_PATHS} )
			message( STATUS ${DIR_TABS} "\t\tAdditional directory: " ${_PATH} )
			set( _ADD_PATHS
				${_ADD_PATHS}
				${_PATH} )
		endforeach()

		_glob_files( ${_GLOB_STYLE}
			${_ADD_PATHS} )

		if( NOT ${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES )
			message( FATAL_ERROR "No files found in directory '" ${${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH} "'" )
		endif()

		set( _ADD_PATHS )
		set( _GLOB_STYLE )
		set( _ADD_PATHS )
	endif()
endmacro()



### _glob_files #######################################
### Adds all files with predefined ending to        ###
### ${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES.       ###
### Uses:                                           ###
### _GLOB_STYLE                                     ###
### _PATH                                           ###
### Sets: ${Project_Name}_${SUB_RELATIV_DIR}_FILES  ###
### Unsets:                                         ###
### _GLOB_STYLE                                     ###
### _PATH                                           ###
#######################################################
macro( _glob_files
		_GLOB_STYLE )

	if( NOT WIN32 )
		set( _FILE_EXTENSIONS
			*.h
			*.hpp
			*.c
			*.cc
			*.cpp
			*.fx
			*.hlsl
			*.glsl
			*.cg
			*.xml
			*.ui
			*.qrc
				*.in )

		set( _ALL_PATH_EXT )
		foreach( _PATH ${ARGN} )
			set( _PATH_EXT )
			foreach( _EXT ${_FILE_EXTENSIONS} )
				list( APPEND _PATH_EXT 
					${_PATH}/${_EXT} )
			endforeach()
			list( APPEND _ALL_PATH_EXT
				${_PATH_EXT} )
		endforeach()
	else()
		set( _FILE_EXTENSIONS
			*.h
			*.hpp
			*.c
			*.cc
			*.cpp )
		set( _QT_FILE_EXTENSIONS
			*.ui
			*.qrc
			*.rcs )
		set( _SHADER_FILE_EXTENSIONS
			*.hlsl
			*.glsl
			*.cg
			*.fx )
		set( _XML_FILE_EXTENSIONS
			*.xml )
		set( _AUTOCONF_FILE_EXTENSIONS
			*.in )

		set( _ALL_PATH_EXT )
		set( _ALL_QT_PATH_EXT )
		set( _ALL_SHADER_PATH_EXT )
		set( _ALL_XML_PATH_EXT )
		set( _ALL_AUTOCONF_PATH_EXT )
		foreach( _PATH ${ARGN} )
			set( _PATH_EXT )
			set( _QT_PATH_EXT )
			set( _SHADER_PATH_EXT )
			set( _XML_PATH_EXT )
			set( _AUTOCONF_PATH_EXT )
			# cpp
			foreach( _EXT ${_FILE_EXTENSIONS} )
				list( APPEND _PATH_EXT
					${_PATH}/${_EXT} )
			endforeach()
			# qt
			foreach( _EXT ${_QT_FILE_EXTENSIONS} )
				list( APPEND _QT_PATH_EXT
					${_PATH}/${_EXT} )
			endforeach()
			# shaders
			foreach( _EXT ${_SHADER_FILE_EXTENSIONS} )
				list( APPEND _SHADER_PATH_EXT
					${_PATH}/${_EXT} )
			endforeach()
			# xml
			foreach( _EXT ${_XML_FILE_EXTENSIONS} )
				list( APPEND _XML_PATH_EXT
					${_PATH}/${_EXT} )
			endforeach()
			# autoconf
			foreach( _EXT ${_AUTOCONF_FILE_EXTENSIONS} )
				list( APPEND _AUTOCONF_PATH_EXT
					${_PATH}/${_EXT} )
			endforeach()

			list( APPEND _ALL_PATH_EXT
				${_PATH_EXT} )
			list( APPEND _ALL_QT_PATH_EXT
				${_QT_PATH_EXT} )
			list( APPEND _ALL_SHADER_PATH_EXT
				${_SHADER_PATH_EXT} )
			list( APPEND _ALL_XML_PATH_EXT
				${_XML_PATH_EXT} )
			list( APPEND _ALL_AUTOCONF_PATH_EXT
				${_AUTOCONF_PATH_EXT} )
		endforeach()

		file( GLOB _QT_FILES
			${_ALL_QT_PATH_EXT} )
		file( GLOB _SHADER_FILES
			${_ALL_SHADER_PATH_EXT} )
		file( GLOB _XML_FILES
			${_ALL_XML_PATH_EXT} )
		file( GLOB _AUTOCONF_FILES
			${_ALL_AUTOCONF_PATH_EXT} )

		source_group( Qt
			FILES ${_QT_FILES} )
		source_group( Shaders
			FILES ${_SHADER_FILES} )
		source_group( XML
			FILES ${_XML_FILES} )
		source_group( Autoconf
			FILES ${_AUTOCONF_FILES} )

		list( APPEND _ALL_PATH_EXT
			${_ALL_QT_PATH_EXT}
			${_ALL_SHADER_PATH_EXT}
			${_ALL_XML_PATH_EXT}
			${_ALL_AUTOCONF_PATH_EXT} )
	endif()

	file( ${_GLOB_STYLE} ${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES
		${_ALL_PATH_EXT} )

	set( _PATH )
	set( _FILE_EXTENSIONS )
	set( _PATH_EXT )
	set( _EXT )
	set( _ALL_PATH_EXT )
	set( _GLOB_STYLE )
	set( _QT_FILES )
	set( _SHADER_FILES )
	set( _XML_FILES )
	set( _AUTOCONF_FILES )
endmacro()



### _check_create_args ################################
### Argument parser for create_executable and       ###
### create_library.                                 ###
###                                                 ###
### Currently used keywords:                        ###
###	LINK                                        ###
###	QTMOC                                       ###
###	QTUI                                        ###
###	QTRCS                                       ###
###	LINK_STATIC                                 ###
###	LINK_SHARED                                 ###
###	ARGUMENTS (for tests)                       ###
###	USE_WINMAIN                                 ###
###	RECURSIVE                                   ###
###	ADD_PATHS                                   ###
###	FILES                                       ###
###	DOXYGEN_CONF (for documentations)           ###
###                                                 ###
### Parameters: List of arguments                   ###
### Uses: nothing                                   ###
### Sets:                                           ###
###	CREATE_LINK_LIBRARIES                       ###
###	CREATE_MOC_FILES                            ###
###	CREATE_UI_FILES                             ###
###	CREATE_RCS_FILES                            ###
###	CREATE_ARGUMENTS                            ###
###	SUB_ADD_INCLUDES                            ###
### 	USE_WIMAIN                                  ###
### 	GLOB_RECURSIVE                              ###
### 	CREATE_ADD_PATHS                            ###
###	USE_LINKING                                 ###
###	CREATE_FILES                                ###
###	CREATE_DOXYGEN_CONFIGURATION                ###
#######################################################
macro( _check_create_args )

	set( CREATE_LINK_LIBRARIES )
	set( CREATE_MOC_FILES )
	set( CREATE_RCS_FILES )
	set( CREATE_ARGUMENTS )
	set( CREATE_UI_FILES )
	set( SUB_ADD_INCLUDES )
	set( USE_WINMAIN )
	set( GLOB_RECURSIVE )
	set( CREATE_ADD_PATHS )
	set( CREATE_FILES )
	set( USE_LINKING )
	set( CREATE_DOXYGEN_CONFIGURATION )
	foreach( _PARAM ${ARGN} )
		string( TOUPPER
			${_PARAM}
			_UPARAM )
		if( _UPARAM STREQUAL "LINK" )
			set( _CREATE_ARG_VAR
				"LINK_LIBRARIES" )
		else()
			if( _UPARAM STREQUAL "QTMOC" )
				set( _CREATE_ARG_VAR
					"MOC_FILES" )
			else()
				if( _UPARAM STREQUAL "QTUI" )
					set( _CREATE_ARG_VAR 
						"UI_FILES" )
				else()
					if( _UPARAM STREQUAL "QTRCS" )
						set( _CREATE_ARG_VAR
							"RCS_FILES" )
					else()
						# Arguments for tests
						if( _UPARAM STREQUAL "ARGUMENTS" )
							set( _CREATE_ARG_VAR
								"ARGUMENTS" )
						else()
							if( _UPARAM STREQUAL "USE_WINMAIN" )
								message( STATUS ${DIR_TABS} "\t\t(using WinMain instead of main)" )
								set( USE_WINMAIN
									"WIN32" )
							else()
								if( _UPARAM STREQUAL "RECURSIVE" )
									set( GLOB_RECURSIVE 
										ON )
								else()
									if( _UPARAM STREQUAL "ADD_PATHS" )
										set( _CREATE_ARG_VAR
											"ADD_PATHS" )
									else()
										if( _UPARAM STREQUAL "FILES" )
											set( _CREATE_ARG_VAR
												"FILES" )
										else()
											if( _UPARM STREQUAL "LINK_STATIC" )
												message( STATUS ${DIR_TABS} "\t\t(linking static)" )
												set( USE_LINKING
													"STATIC" )
											else()
												if( _UPARAM STREQUAL "LINK_SHARED" )
													message( STATUS ${DIR_TABS} "\t\t(linking shared)" )
													set( USE_LINKING
														"SHARED" )
												else()
													if( _UPARAM STREQUAL "DOXYGEN_CONF" )
														set( _CREATE_ARG_VAR
															"DOXYGEN_CONFIGURATION" )
													else()
														# if everything failed, report an error
														if( _CREATE_ARG_VAR STREQUAL "" )
															message( FATAL_ERROR
																"Error while parsing arguments for create_*: No keyword as first argument!" )
														else()
															# create variable
															set( CREATE_${_CREATE_ARG_VAR}
																${CREATE_${_CREATE_ARG_VAR}}
																${_PARAM} )
														endif()
													endif()
												endif()
											endif()
										endif()
									endif()
								endif()
							endif()
						endif()
					endif()
				endif()
			endif()
		endif()
	endforeach()
	set( _PARAM )
	set( _UPARAM )
	set( _CREATE_ARG_VAR )
endmacro()



### _add_all_moc ######################################
### Runs QT4_WRAP_CPP on given headers.             ###
###                                                 ###
### Parameters: List of headers with Q_OBJECT       ###
### Uses:                                           ###
###	${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH     ###
### Sets: WRAPPED_MOC_FILES                         ###
#######################################################
macro( _add_all_moc )

	set( _CREATE_MOC_FILES_WITH_DIR )
	foreach( _MOC_FILE ${ARGN} )
		if( EXISTS ${${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH}/${_MOC_FILE} )
			list( APPEND _CREATE_MOC_FILES_WITH_DIR
				${${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH}/${_MOC_FILE} )
		else()
			message( FATAL_ERROR "A specified Qt moc file does not exist: " ${${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH}/${_MOC_FILE} )
		endif()
	endforeach()
	if( NOT _CREATE_MOC_FILES_WITH_DIR )
		message( FATAL_ERROR "QTMOC specified, but no files given." )
	endif()
	QT4_WRAP_CPP( WRAPPED_MOC_FILES
		${_CREATE_MOC_FILES_WITH_DIR} )
	set( _CREATE_MOC_FILES_WITH_DIR )
	set( _MOC_FILE )
endmacro()



### _add_all_ui #######################################
### Runs QT4_WRAP_UI on given .ui files.            ###
### Note that .ui files can be in a subdirectory of ###
### ${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH or  ###
### ${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH.    ###
###                                                 ###
### Parameters: List of .ui files                   ###
### Uses:                                           ###
###	${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_DIR      ###
### Sets: WRAPPED_UI_FILES                          ###    
#######################################################
macro( _add_all_ui )

	include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
	set( _CREATE_UI_FILES_WITH_DIR )
	foreach( _UI_FILE ${ARGN} )
		if( EXISTS ${${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH}/${_UI_FILE} )
			list( APPEND _CREATE_UI_FILES_WITH_DIR
				${${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH}/${_UI_FILE} )
		else()
			if( EXISTS ${${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH}/${_UI_FILE} )
				list( APPEND _CREATE_UI_FILES_WITH_DIR
					${${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH}/${_UI_FILE} )
			else()
				message( FATAL_ERROR "A specified Qt ui file does not exist: " ${${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH}/${_UI_FILE} )
			endif()
		endif()

	endforeach()
	if( NOT _CREATE_UI_FILES_WITH_DIR )
		message( FATAL_ERROR "QTUI specified, but no files given." )
	endif()
	
	
	QT4_WRAP_UI( WRAPPED_UI_FILES
		${_CREATE_UI_FILES_WITH_DIR} )
	set( _CREATE_UI_FILES_WITH_DIR )
	set( _UI_FILE )
endmacro()



### _add_all_rcs ######################################
### Runs QT4_ADD_RESOURCES on given files.          ###
### Note that these files can be in a subdirectory  ###
### of ${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH. ###
###                                                 ###
### Parameters: List of resource files              ###
### Uses:                                           ###
###	${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_DIR      ###
### Sets: ADDED_RCS_FILES                           ###
#######################################################
macro( _add_all_rcs )

	set( _CREATE_RCS_FILES_WITH_DIR )
	foreach( _RCS_FILE ${ARGN} )
		if( NOT EXISTS ${${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH}/${_UI_FILE} )
			message( FATAL_ERROR "A specified Qt qrc file does not exist: " ${${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH}/${_UI_FILE} )
		endif()
		list( APPEND _CREATE_RCS_FILES_WITH_DIR
			${${PROJECT_NAME}_${SUB_RELATIV_DIR}_SRC_PATH}/${_RCS_FILE} )
	endforeach()
	if( NOT _CREATE_RCS_FILES_WITH_DIR )
		message( FATAL_ERROR "QTRCS specified, but no files given." )
	endif()
	QT4_ADD_RESOURCES( ADDED_RCS_FILES
		${_CREATE_RCS_FILES_WITH_DIR} )
	set( _CREATE_RCS_FILES_WITH_DIR )
	set( _RCS_FILE )
endmacro()



### create_executable #################################
### Wrapper for add_executable. Creates an          ###
### executable _EXEC_NAME, runs QT4 macros if       ###
### needed and links against privided libraries.    ###
###                                                 ###
### Parameters: List of arguments (parsed by        ###
###		_check_create_args)                         ###
### Uses:                                           ###
###	WRAPPED_MOC_FILES                               ###
###	WRAPPED_UI_FILES                                ###
###	ADDED_RCS_FILES                                 ###
###	${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES        ###
###	CREATE_LINK_LIBRARIES                           ###
### Sets: nothing                                   ###
### Unsets:                                         ###
###	WRAPPED_MOC_FILES                               ###
###	WRAPPED_UI_FILES                                ###
###	ADDED_RCS_FILES                                 ###
#######################################################
macro( create_executable
	_EXEC_NAME)

	message( STATUS ${DIR_TABS} "\tAdding executable: " ${_EXEC_NAME} )
	_create_target( ${ARGN} )
	add_executable( ${_EXEC_NAME}
		${USE_WINMAIN}
		${WRAPPED_MOC_FILES}
		${WRAPPED_UI_FILES}
		${ADDED_RCS_FILES}
		${${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES} )
	target_link_libraries( ${_EXEC_NAME}
		${CREATE_LINK_LIBRARIES} )

	# Copy boost dll to bin/
	if( WIN32 AND
	    ${LIBRARY_TYPE} STREQUAL "SHARED" )
		_copy_boost_dll( ${_EXEC_NAME} )
	endif()

	set( _EXEC_NAME )
	set( WRAPPED_MOC_FILES )
	set( WRAPPED_UI_FILES )
	set( ADDED_RCS_FILES )
	set( USE_WINMAIN )
endmacro()



### create_library ####################################
### Wrapper for add_library. Creates a library      ###
### _LIB_NAME, runs QT4 macros if needed and links  ###
### against provided libraries.                     ###
### If a shared library on windows is created, the  ###
### script will copy the .dll in the binary         ###
### directory.                                      ###
###                                                 ###
### Parameters: List of arguments (parsed by        ###
###		_check_create_args)                         ###
### Uses:                                           ###
###	WRAPPED_MOC_FILES                               ###
###	WRAPPED_UI_FILES                                ###
###	ADDED_RCS_FILES                                 ###
###	${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES        ###
###	CREATE_LINK_LIBRARIES                           ###
###	WIN32 (cmake)                                   ###
###	LIBRARY_TYPE                                    ###
### Sets: nothing                                   ###
### Unsets:                                         ###
### WRAPPED_MOC_FILES                               ###
### WRAPPED_UI_FILES                                ###
### ADDED_RCS_FILES                                 ###
#######################################################
macro( create_library
	_LIB_NAME )

	message( STATUS ${DIR_TABS} "\tAdding library: " ${_LIB_NAME} )
	_create_target( ${ARGN} )
	
	set( LIB_LINK_TYPE
		${LIBRARY_TYPE} )
	if( ${USE_LINKING} )
		set( LIB_LINK_TYPE
			${USE_LINKING} )
	endif()
	add_library( ${_LIB_NAME} ${LIB_LINK_TYPE}
		${WRAPPED_MOC_FILES}
		${WRAPPED_UI_FILES}
		${ADDED_RCS_FILES}
		${${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES} )
	target_link_libraries( ${_LIB_NAME}
		${CREATE_LINK_LIBRARIES} )
	if( WIN32 AND
	    ${LIBRARY_TYPE} STREQUAL "SHARED" )
		_create_library_copy_dll( ${_LIB_NAME} )
	endif()
	set( _LIB_NAME )
	set( WRAPPED_MOC_FILES )
	set( WRAPPED_UI_FILES )
	set( ADDED_RCS_FILES )
endmacro()



### create_test #######################################
### Wrapper for add_test. Enables testing in cmake  ###
### and creates a test _TEST_NAME. Arguments can be ###
### provided.                                       ###
###                                                 ###                
### Parameters: List of arguments (parsed by        ###
### 	_check_create_args)                         ###
### Uses:                                           ###
### ${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES        ###
### CREATE_LINK_LIBRARIES                           ###
### USE_WINMAIN                                     ###
### WRAPPED_MOC_FILES                               ###
### WRAPPED_UI_FILES                                ###
### ADDED_RCS_FILES                                 ###
### Sets: nothing                                   ###
### Unsets:                                         ###
### USE_WINMAIN                                     ###
### WRAPPED_MOC_FILES                               ###
### WRAPPED_UI_FILES                                ###
### ADDED_RCS_FILES                                 ###
#######################################################
macro( create_test
	_TEST_NAME )

	message( STATUS ${DIR_TABS} "\tAdding test: " ${_TEST_NAME} )
	
	# also use enable_testing in top level CMakeLists to create a test project
	enable_testing()
	_create_target( ${ARGN} )
	add_executable( ${_TEST_NAME}
		${USE_WINMAIN}
		${WRAPPED_MOC_FILES}
		${WRAPPED_UI_FILES}
		${ADDED_RCS_FILES}
		${${PROJECT_NAME}_${SUB_RELATIV_DIR}_FILES} )
	target_link_libraries( ${_TEST_NAME}
		${CREATE_LINK_LIBRARIES} )
	add_test( TEST_${_TEST_NAME}
		  ${EXECUTABLE_OUTPUT_PATH}/${_TEST_NAME}
		  ${CREATE_ARGUMENTS} )
	set( _TEST_NAME )
	set( WRAPPED_MOC_FILES )
	set( WRAPPED_UI_FILES )
	set( ADDED_RCS_FILES )
	set( USE_WINMAIN )
endmacro()



### create_documentation ##############################
### Custom target for building a doxygen            ###
### documentation. Adds target ${_DOC_NAME}Doc for  ###
### building and clean${_DOC_NAME}Doc for cleaning  ###
### the documentation.                              ###
### Parameters: target name                         ###
### Uses:                                           ###
### DOXYGEN_EXECUTABLE                              ###
### CREATE_DOXYGEN_CONFIGURATION                    ###
### Sets: nothing                                   ###
### Unsets: CREATE_DOXYGEN_CONFIGURATION            ###
#######################################################
macro( create_documentation
	_DOC_NAME )

	if( NOT DOXYGEN_EXECUTABLE )
		message( FATAL_ERROR "Doxygen executable not found!" )
	endif()
	_create_target( ${ARGN} )
	if( NOT CREATE_DOXYGEN_CONFIGURATION )
		message( FATAL_ERROR "Please specify a configuration file for doxygen." )
	endif()

	message( STATUS ${DIR_TABS} "\tAdding documentation: ${_DOC_NAME}" )
	configure_file( ${CREATE_DOXYGEN_CONFIGURATION}
		${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	   	@ONLY )
	add_custom_target( ${_DOC_NAME}
		${DOXYGEN_EXECUTABLE}
	    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating documentation with doxygen"
		VERBATIM )

	message( STATUS ${DIR_TABS} "\tAdding clean target: clean${_DOC_NAME}" )
	add_custom_target( clean${_DOC_NAME} )
	add_custom_command( TARGET clean${_DOC_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${DOC_OUTPUT_PATH}/html
		COMMENT "Deleting documentation ${DOC_OUTPUT_PATH}/html"
		VERBATIM )

	set( CREATE_DOXYGEN_CONFIGURATION )
	set( _DOC_NAME )
endmacro()



### _create_library_copy_dll ##########################
### Copy .dll to binary directory by creating a     ###
### POSTBUILD target.                               ###
### Creates target directory like 'build/Debug'.    ###
###                                                 ###
### Parameters: _LIB_NAME                           ###
### Uses:                                           ###
###	MSVC_VERSION (cmake)                            ###
###	EXECUTABLE_OUTPUT_PATH                          ###
###	CMAKE_COMMAND (cmake)                           ###
###	$(Configuration) (Visual Studio 2010)           ###
###	$(OutDir) (Visual Studio 2008 and below)        ###
### Sets: nothing                                   ###
#######################################################
macro( _create_library_copy_dll
	_LIB_NAME )

	get_target_property( _LIB_NAME_WITH_PATH
		${_LIB_NAME} LOCATION )

	if( ${MSVC_VERSION} VERSION_GREATER 1500 ) # if VS10 or higher
		set( _DLL_COPYTO_PATH
			${EXECUTABLE_OUTPUT_PATH}/\$\(Configuration\) )
	else()
		set( _DLL_COPYTO_PATH
			${EXECUTABLE_OUTPUT_PATH}/\$\(OutDir\) )
	endif()

	add_custom_command( TARGET ${_LIB_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${_DLL_COPYTO_PATH} && ${CMAKE_COMMAND} -E copy ${_LIB_NAME_WITH_PATH} ${_DLL_COPYTO_PATH}/
		COMMENT "Copying " ${_LIB_NAME} ".dll to " ${_DLL_COPYTO_PATH}/
		VERBATIM )
	set( _LIB_NAME )
	set( _LIB_NAME_WITH_PATH )
	set( _DLL_COPYTO_PATH )
endmacro()



### _copy_boost_dll #####################################
### copy needed boost dll to bin/                     ###
### (Windows shared only)                             ###
###                                                   ###
### Parameters: _LIB_NAME                             ###
### Uses:                                             ###
###	CREATE_LINK_LIBRARIES                             ###
###	MSVC_VERSION (cmake)                              ###
###	EXECUTABLE_OUTPUT_PATH                            ###
###	$(CONFIGURATION) (Visual Studio 2010)             ###
###	$(OutDir) (Visual Studio 2008 and below)          ###
###	CMAKE_COMMAND (cmake)                             ###
### Sets: nothing                                     ###
#########################################################
macro( _copy_boost_dll 
	_LIB_NAME )

	set( _LIB_BOOST )
	foreach( _BOOST_LIBRARY ${CREATE_LINK_LIBRARIES} )
		if( _BOOST_LIBRARY MATCHES "boost_" )
			string( REGEX REPLACE
				".lib$" ".dll"
				_BOOST_DLL
				${_BOOST_LIBRARY} )
				if( ${MSVC_VERSION} VERSION_GREATER 1500 ) # if VS10 or higher
					set( _DLL_COPYTO_PATH
						${EXECUTABLE_OUTPUT_PATH}/\$\(Configuration\) )
				else()
					set( _DLL_COPYTO_PATH
						${EXECUTABLE_OUTPUT_PATH}/\$\(OutDir\) )
				endif()

			add_custom_command( TARGET ${_LIB_NAME}
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy ${_BOOST_DLL} ${_DLL_COPYTO_PATH}
				VERBATIM )
		endif()
	endforeach()
endmacro()



### _create_target ####################################
### Used by create_executable and create_library to ###
### call _check_create_args, _add_all_files and run ###
### QT4 wrapper macros.                             ###
###                                                 ###
### Parameter: List of arguments (parsed by         ###
###		_check_create_args)                         ###
### Uses:                                           ###
###	CREATE_MOC_FILES                                ###
###	CREATE_UI_FILES                                 ###
###	CREATE_RCS_FILES                                ###
### Sets: nothing                                   ###
### Unsets:                                         ###
###	CREATE_MOC_FILES                                ###
###	CREATE_UI_FILES                                 ###
###	CREATE_RCS_FILES                                ###
#######################################################
macro( _create_target )

	_check_create_args( ${ARGN} )
	_add_all_files()
	include_directories( ${${PROJECT_NAME}_${SUB_RELATIV_DIR}_INC_PATH} )
	if( DEFINED CREATE_MOC_FILES )
		_add_all_moc( ${CREATE_MOC_FILES} )
	endif()
	if( DEFINED CREATE_UI_FILES )
		_add_all_ui( ${CREATE_UI_FILES} )
	endif()
	if( DEFINED CREATE_RCS_FILES )
		_add_all_rcs( ${CREATE_RCS_FILES} )
	endif()
	set( CREATE_MOC_FILES )
	set( CREATE_UI_FILES )
	set( CREATE_RCS_FILES )
endmacro()



### print_options #####################################
### Displays the status of given option variables   ###
### Uses: nothing                                   ###
### Sets: nothing                                   ###
### Unsets: nothing                                 ###
#######################################################
macro( print_options )
	
	message( STATUS )
	message( STATUS "Options:" )
	foreach( _OPT ${ARGN} )
		message( STATUS "\t" ${_OPT} ": " ${${_OPT}} )
	endforeach()
	message( STATUS )
	set( _OPT )
endmacro()
