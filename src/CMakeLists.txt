cmake_minimum_required(VERSION 3.0)

PROJECT(Doppelganger)
SET(CMAKE_CXX_STANDARD 17)

######################## EDIT IF REQUIRED ####################
# ###Uncomment the line below and specify the path to OpenCV directory i.e. the path to the OpenCVConfig.cmake file. Check the examples given below.
#SET(OpenCV_DIR Enter-the-path-of-OpenCV-installation-on-your-system)
set(OpenCV_DIR /opt/opencv/4.5.1/installation/lib/cmake/opencv4)


################### OpenCV_DIR Examples  #####################

### MACOS : /usr/local/Cellar/opencv/3.3.1_1/share/OpenCV/

### UBUNTU : /usr/local/share/OpenCV/

### WINDOWS : C:\Users\yourname\Documents\opencv-3.3.1\build\install

##############################################################




if(MSVC)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

find_package( OpenCV REQUIRED )


include(../dlib/dlib/cmake)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(doppelganger 
	main.cpp 
	facedb.h
	facedb_impl.h
	dnnfacedescriptorcomputer.h
	resnetfacedescriptorcomputer.h
	resnet.h
	dlibfaceextractor.h
)

#[[
# GCC requires tbb to be linked in order to use execution policies
set(LINK_LIBS ${OpenCV_LIBS} dlib::dlib)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")    
    list(APPEND LINK_LIBS tbb)
endif()

#message(${LINK_LIBS})
target_link_libraries(doppelganger ${LINK_LIBS})
]]
target_link_libraries(doppelganger ${OpenCV_LIBS} dlib::dlib)
		
	
# TODO: Copy Dlib models to the output directory
#[[
add_custom_command(TARGET doppelganger
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     "${PROJECT_SOURCE_DIR}/../models/*"
                     "$<TARGET_FILE_DIR:doppelganger>")
]]

# TODO: Copy celeba mini dataset to the output directory
#[[
add_custom_command(TARGET doppelganger
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   "${PROJECT_SOURCE_DIR}/../dataset"
                   "$<TARGET_FILE_DIR:doppelganger>/dataset")
]]
	
# Copy test image files to the output directory
add_custom_command(TARGET doppelganger
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   "${PROJECT_SOURCE_DIR}/../test"
                   "$<TARGET_FILE_DIR:doppelganger>/test")

