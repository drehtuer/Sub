if(WIN32)
	# requires the envionment variable DXSDK_DIR to be set
	set( SDK_SEARCH_DIR
		"$ENV{DXSDK_DIR}" )

	message(STATUS "ENV: " ${SDK_SEARCH_DIR})

	find_path( DirectX_INCLUDE_DIR 
		DxErr.h
		D3DX10.h
		PATH ${SDK_SEARCH_DIR}/Include )
#	message( STATUS "DirectX INC: " ${DirectX_INCLUDE_DIR} )

	set( DX_LIBS
		D3DCSX
		d2d1
		d3dx10
		d3dxof
		xapobase
		D3DCSXd
		d3d10
		d3dx10d
		dinput8
		xapobased
		DxErr
		d3d10_1
		d3dx11
		dsound
		X3DAudio
		d3d11
		d3dx11d
		dwrite
		XAPOFX
		d3d9
		d3dx9
		dxgi
		XInput
		d3dcompiler
		d3dx9d
		dxguid
		DxErr
		d3d10 )

	set( DirectX_LIBRARIES 
		"" )
	foreach( LIB ${DX_LIBS} )
		find_library( DX_${LIB}
			${LIB}
			PATH ${SDK_SEARCH_DIR}/Lib/x64 )
		set( DirectX_LIBRARIES
			${DirectX_LIBRARIES}
			${DX_${LIB}} )
	endforeach()

#	message( STATUS "DirectX LIBS: " ${DirectX_LIBRARIES} )

else()
	message( ERROR "No DirectX support on non Windows platforms" )
endif()

include( FindPackageHandleStandardArgs )

find_package_handle_standard_args( DirectX
	DEFAULT_MSG
	DirectX_INCLUDE_DIR
	DirectX_LIBRARIES )
