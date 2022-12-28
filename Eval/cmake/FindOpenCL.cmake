message( STATUS "CMake OpenCL support currently only for Windows & nVidia" )

if(WIN32)

	set( SDK_SEARCH_DIR
		"$ENV{CUDA_PATH}" )

	#message( STATUS "dir: " ${SDK_SEARCH_DIR})

	find_path( OPENCL_INCLUDE_DIR
		CL/cl.h
		PATH ${SDK_SEARCH_DIR}/include )
	find_library( OPENCL_LIBRARIES
		OpenCL
		PATH ${SDK_SEARCH_DIR}/lib/x64 )
	#message( STATUS "inc: " ${OPENCL_INCLUDE_DIR})
	#message( STATUS "lib: " ${OPENCL_LIBRARIES})
else()

endif()

include( FindPackageHandleStandardArgs )

find_package_handle_standard_args( OpenCL
	DEFAULT_MSG
	OPENCL_INCLUDE_DIR
	OPENCL_LIBRARIES )
