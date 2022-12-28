if( OpenCV_FOUND )
	create_executable( NormalMapRH2LH
		LINK ${OpenCV_LIBS} )
endif()
