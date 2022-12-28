if(WIN32)
	set( NVIDIA_COMPUTE_ROOT
		$ENV{NVSDKCOMPUTE_ROOT} )
	set( NVIDIA_CUDA_ROOT
		$ENV{NVSDKCUDA_ROOT} )
#	message( STATUS "cuda: " ${NVIDIA_CUDA_ROOT} ", compute: " ${NVIDIA_COMPUTE_ROOT} )
	set( NVIDIA_COMMON_DIR 
		false )
	if( NVIDIA_CUDA_ROOT )
		set( NVIDIA_COMMON_DIR
			${NVIDIA_CUDA_ROOT}/common)
	else()
		if( NVIDIA_COMPUTE_ROOT )
			set( NVIDIA_COMMON_DIR
				${NVIDIA_COMPUTE_ROOT}/C/common )
		endif()
	endif()

	if( NOT NVIDIA_COMMON_DIR )
		message( ERROR " Install NVIDIA CUDA SDK Samples, it contains GLEW headers and libraries" )
	endif()
	
	find_path( GLEW_INCLUDE_DIR
		GL/glew.h
		PATH ${NVIDIA_COMMON_DIR}/inc )
	
	if(CMAKE_CL_64)
		find_library( GLEW_LIBRARIES
			glew64
			PATH ${NVIDIA_COMMON_DIR}/lib )
	else()
		find_library( GLEW_LIBRARIES
			glew32
			PATH ${NVIDIA_COMMON_DIR}/lib )
	endif()

else() # unix & co
	find_path( GLEW_INCLUDE_DIR
		GL/glew.h
		PATH /usr/include )
	if( APPLE )
        find_library( GLEW_LIBRARIES
            libGLEW.dylib
            PATH /usr/lib
                 /opt/local/lib )
    else()
        find_library( GLEW_LIBRARIES
		libGLEW.so
		PATH /usr/lib )
    endif()
endif()

#message( STATUS "GLEW include dir: " ${GLEW_INCLUDE_DIR} )
#message( STATUS "GLEW library dir: " ${GLEW_LIBRARIES} )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( GLEW
	DEFAULT_MSG
	GLEW_INCLUDE_DIR
	GLEW_LIBRARIES )
