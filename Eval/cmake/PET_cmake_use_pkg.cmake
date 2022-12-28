#######################################################
### Collection of wrapped find_package commands.    ###
### Simply include this file and PET_cmake_tools    ###
### in your CMakeList.txt.                          ###
### Make sure to add the path of this file to your  ###
### CMAKE_MODULE_PATH.                              ###
###                                                 ###
### Add a (supported) package A to your project by: ###
###	use_package_A( [REQUIRED|OPTIONAL]          ###
###		[COMPONENTS component1 ...]         ###
###		[VERSION major[.minor[.patch]]] )   ###
###                                                 ###
### Don't forget to link your targets against all   ###
### needed libraries.                               ###
#######################################################



### OpenGL ############################################
### Components:                                     ###
###	OPENGL_XMESA                                ###
###	OPENGL_GLU                                  ###
### Returns:                                        ###
###	OPENGL_FOUND                                ###
###	OPENGL_INCLUDE_DIR (already included)       ###
###	OPENGL_LIBRARIES                            ###
###	OPENGL_gl_LIBRARY                           ###
###	OPENGL_glu_LIBRARY                          ###
#######################################################
macro( use_pkg_opengl )

	_check_pkg_args( ${ARGN} )
	find_package( OpenGL QUIET )
	_check_pkg_found( OPENGL )
	_check_pkg_version( OpenGL )
	_check_pkg_components( ${PKG_COMPONENTS} )
	_check_pkg_includes( ${OPENGL_INCLUDE_DIR} )
endmacro()



### OpenCV ############################################
### Components: none                                ###
### Returns:                                        ###
###	OpenCV_FOUND                                ###
###	OpenCV_LIBS                                 ###
###	OpenCV_LIB_DIR                              ###
###	OpenCV_INCLUDE_DIRS (already included)      ###
#######################################################
macro( use_pkg_opencv )

	_check_pkg_args( ${ARGN} )
	find_package( OpenCV QUIET )
	_check_pkg_found( OpenCV )
	_check_pkg_version( OpenCV ${OpenCV_VERSION} )
	_check_pkg_includes( ${OpenCV_INCLUDE_DIRS} )
endmacro()



### BOOST #############################################
### Components:                                     ###
###	date_time                                   ###
###	thread                                      ###
###	and more ...                                ###
### Returns:                                        ###
###	Boost_FOUND                                 ###
###	Boost_INCLUDE_DIRS (already included)       ###
###	Boost_INCLUDE_DIR                           ###
###	Boost_LIBRARIES                             ###
###	Boost_LIBRARY_DIRS                          ###
#######################################################
macro( use_pkg_boost )

	set( Boost_MULTITHREADED ON )
	set( Boost_USE_STATIC_LIBS ON )

	_check_pkg_args( ${ARGN} )
	find_package( Boost COMPONENTS ${PKG_COMPONENTS} QUIET )
	_check_pkg_found( Boost )
	_check_pkg_version( Boost ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} )
	foreach( COMPONENT ${PKG_COMPONENTS} )
		string( TOUPPER
			${COMPONENT}
			UPPER_COMPONENT )
		set( BOOSTBOOST_COMPONENTS
			${BOOSTBOOST_COMPONENTS}
			Boost_${UPPER_COMPONENT} )
	endforeach()
	_check_pkg_components( ${BOOSTBOOST_COMPONENTS} )
	_check_pkg_includes( ${Boost_INCLUDE_DIRS} )
	if( WIN32 )
		link_directories( ${Boost_LIBRARY_DIRS} )
	endif()
endmacro()



### PET ###############################################
### Components: none                                ###
### Returns:                                        ###
###	PET_FOUND                                   ###
###	PTC_FOUND                                   ###
###	LIBPET_INC_DIR (already included)           ###
###	LIBPTC_INC_DIR (already included)           ###
###	LIBPET_LIBRARY                              ###
###	LIBPETGUI_LIBRARY                           ###
###	LIBPTC_LIBRARY                              ###
#######################################################
macro( use_pkg_pet )

	_check_pkg_args( ${ARGN} )
	find_package( PET QUIET )
	_check_pkg_found( PET )
	_check_pkg_version( PET )
	_check_pkg_found( PTC )
	_check_pkg_version( PTC )
	_check_pkg_includes( ${LIBPET_INC_DIR} ${LIBPTC_INC_DIR} )
endmacro()



#### modPET ###########################################
### Components: none                                ###
### Returns:                                        ###
###	MODPET_FOUND                                ###
###	modPET_INCLUDE_DIR (already included)       ###
#######################################################
macro( use_pkg_modpet )

	_check_pkg_args( ${ARGN} )
	find_package( modPET QUIET )
	_check_pkg_found( MODPET )
	_check_pkg_version( modPET )
	_check_pkg_includes( ${modPET_INCLUDE_DIR} )
endmacro()



### QT4 ###############################################
### Components:                                     ###
###	QT3SUPPORT                                  ###
###	QASSISTANT                                  ###
###	AXCONTAINER                                 ###
###	QAXSERVER                                   ###
###	QTDESIGNER                                  ###
###	QTMOTIF                                     ###
###	QTMAIN                                      ###
###	QTMULTIMEDIA                                ###
###	QTNETWORK                                   ###
###	QTNSPLUGIN                                  ###
###	QTOPENGL                                    ###
###	QTSQL                                       ###
###	QTXML                                       ###
###	QTSVG                                       ###
###	QTTEST                                      ###
###	QTUITOOLS                                   ###
###	QTDBUS                                      ###
###	QTSCRIPT                                    ###
###	ASSISTANTCLIENT                             ###
###	QTHELP                                      ###
###	QTWEBKIT                                    ###
###	QTXMLPATTERNS                               ###
###	PHONON                                      ###
###	QTSCRIPTTOOLS                               ###
###	QTDECLARATIVE                               ###
### Returns:                                        ###
###	QT_USE_FILE (already included)              ###
###	QT_INCLUDE_DIR (already included)           ###
###	QT_LIBRARIES                                ###
###	and more ...                                ###
### Macros:                                         ###
###	QT4_WRAP_CPP                                ###
###	QT4_WRAP_UI                                 ###
###	QT4_ADD_RESOURCES                           ###
###	and more ...                                ###
#######################################################
macro( use_pkg_qt4 )
	_check_pkg_args( ${ARGN} )
	foreach( COMPONENT ${PKG_COMPONENTS} )
		set( QT_USE_${COMPONENT}
			ON )
	endforeach()

	find_package( Qt4 QUIET )
	_check_pkg_found( QT4 )
	_check_pkg_version( Qt4 ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH} )
	foreach( COMPONENT ${PKG_COMPONENTS} )
		string( TOUPPER
			${COMPONENT}
			QT_COMPONENT )
		set( QT_COMPONENTS
			${QT_COMPONENTS}
			QT_${QT_COMPONENT} )
	endforeach()
	_check_pkg_components( ${QT_COMPONENTS} )
	include( ${QT_USE_FILE} )
	include_directories( ${CMAKE_BINARY_DIR} ) # for ui
	_check_pkg_includes( ${QT_INCLUDE_DIR} )
endmacro()



### CUDA ##############################################
### Components: none                                ###
### Returns:                                        ###
###	CUDA_FOUND                                  ###
###	CUDA_TOOLKIT_ROOT                           ###
###	CUDA_SDK_ROOT_DIR                           ###
###	CUDA_INCLUDE_DIRS (already included)        ###
###	CUDA_LIBRARIES                              ###
###	CUDA_CUFFT_LIBRARIES                        ###
###	CUDA_CUBLAS_LIBRARIES                       ###
### CUDA Macros:                                    ###
###	CUDA_ADD_CUFFT_TO_TARGET                    ###
###	CUDA_ADD_CUBLAS_TARGET                      ###
###	CUDA_ADD_EXECUTABLE                         ###
###	CUDA_ADD_LIBRARY                            ###
###	CUDA_BUILD_CLEAN_TARGET                     ###
###	CUDA_COMPILE                                ###
###	CUDA_COMPILE_PIX                            ###
###	CUDA_INCLUDE_DIRECTORIES                    ###
###	CUDA_WRAP_SRCS                              ###
#######################################################
macro( use_pkg_cuda )

	_check_pkg_args( ${ARGN} )
	find_package( CUDA QUIET )
	_check_pkg_found( CUDA )
	_check_pkg_version( CUDA ${CUDA_VERSION_STRING} )
	_check_pkg_includes( ${CUDA_INCLUDE_DIRS} )
endmacro()



### Matlab ############################################
### Components: none                                ###
### Returns:                                        ###
###	MATLAB_FOUND                                ###
###	MATLAB_INCLUDE_DIR (already included)       ###
###	MATLAB_LIBRARIES                            ###
###	MATLAB_MEX_LIBRARY                          ###
###	MATLAB_MX_LIBRARY                           ###
###	MATLAB_ENG_LIBRARY                          ###
###	MATLAB_ROOT                                 ###
###	MATLAB_LIB_DIR                              ###
###	MATLAB_MAT_LIBRARY                          ###
#######################################################
macro( use_pkg_matlab )

	_check_pkg_args( ${ARGN} )
	find_package( MATLAB QUIET )
	_check_pkg_found( MATLAB )
	_check_pkg_version( Matlab )
	_check_pkg_includes( ${MATLAB_INCLUDE_DIR} )
endmacro()



### Doxygen ###########################################
### Components:                                     ###
###	DOXYGEN_SKIP_DOT                            ###
### Returns:                                        ###
###	DOXYGEN_EXECUTABLE                          ###
###	DOXYGEN_FOUND                               ###
###	DOXYGEN_DOT_EXECUTABLE                      ###
###	DOXYGEN_DOT_FOUND                           ###
###	DOXYGEN_DOT_PATH                            ###
#######################################################
macro( use_pkg_doxygen )

	_check_pkg_args( ${ARGN} )
	foreach( COMPONENT ${PKG_COMPONENTS} )
		set( ${COMPONENT} true )
	endforeach()
	find_package( Doxygen QUIET )
	_check_pkg_found( DOXYGEN )
	_check_pkg_version( Doxygen )
endmacro()



### OpenCL ############################################
### Components: none                                ###
### Returns:                                        ###
###	OPENCL_FOUND                                ###
###	OPENCL_INCLUDE_DIRS (already included)      ###
###	OPENCL_LIBRARIES                            ###
#######################################################
macro( use_pkg_opencl )

	_check_pkg_args( ${ARGN} )
	find_package( OpenCL QUIET )
	_check_pkg_found( OPENCL )
	_check_pkg_version( OPENCL )
	_check_pkg_includes( ${OPENCL_INCLUDE_DIRS} )
endmacro()



### DirectX ###########################################
### Components: none                                ###
### Returns:                                        ###
### DIRECTX_FOUND                                   ###
### DirectX_INCLUDE_DIR (already included)          ###
### DirectX_LIBRARIES                               ###
#######################################################
macro( use_pkg_directx )

	_check_pkg_args( ${ARGN} )
	find_package( DirectX QUIET )
	_check_pkg_found( DIRECTX )
	_check_pkg_version( DirectX )
	_check_pkg_includes( ${DirectX_INCLUDE_DIR} )
endmacro()



### GLEW ##############################################
### Components: none                                ###
### Returns:                                        ###
### GLEW_FOUND                                      ###
### GLEW_INCLUDE_DIR (already included)             ###
### GLEW_LIBRARIES                                  ###
#######################################################
macro( use_pkg_glew )

	_check_pkg_args( ${ARGN} )
	find_package( GLEW )
	_check_pkg_found( GLEW )
	_check_pkg_version( GLEW )
	_check_pkg_includes( ${GLEW_INCLUDE_DIR} )
endmacro()



### Eigen ##############################################
### Components: none                                 ###
### Returns:                                         ###
### Eigen_FOUND                                      ###
### Eigen_INCLUDE_DIR (already included)             ###
########################################################
macro( use_pkg_eigen )

	_check_pkg_args( ${ARGN} )
	find_package( Eigen )
	_check_pkg_found( EIGEN )
	_check_pkg_version( Eigen )
	_check_pkg_includes( ${Eigen_INCLUDE_DIR} )
endmacro()



macro( use_pkg_anttweakbar )
	_check_pkg_args( ${ARGN} )
	find_package( AntTweakBar )
	_check_pkg_found( ANTTWEAKBAR )
	_check_pkg_version( AntTweakBar )
	_check_pkg_includes( ${AntTweakBar_INCLUDE_DIR} )
endmacro()



macro( use_pkg_assimp )
	_check_pkg_args( ${ARGN} )
	find_package( Assimp )
	_check_pkg_found( ASSIMP )
	_check_pkg_version( Assimp )
	_check_pkg_includes( ${Assimp_INCLUDE_DIR} )
endmacro()

