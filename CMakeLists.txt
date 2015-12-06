# Hold Your Past

# version and project
cmake_minimum_required(VERSION 3.0)
project( HoldYourPast )

# globals variables
set( BIN_NAME 			hold-your-past )
set( EXECUTABLE_OUTPUT_PATH 	${PROJECT_SOURCE_DIR}/bin )

#  files
set( SOURCES_PATH 		src )
set( HEADERS_PATH		headers )
file( GLOB SOURCES 		${SOURCES_PATH}/*.cpp )
file( GLOB HEADERS 		HEADERS_PATH/*.hpp )

# linking the opencv library
find_package( OpenCV REQUIRED )
add_executable( ${BIN_NAME} ${SOURCES} ${HEADERS} )
target_link_libraries( ${BIN_NAME} ${OpenCV_LIBS} )
