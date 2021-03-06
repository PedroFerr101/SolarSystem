cmake_minimum_required(VERSION 3.5)

# Project Name
PROJECT(cg-projeto CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if  (WIN32)

	message(STATUS "Toolkits_DIR set to: " ${TOOLKITS_FOLDER})
	set(TOOLKITS_FOLDER "" CACHE PATH "Path to Toolkits folder")
	
	if (NOT EXISTS "${TOOLKITS_FOLDER}/glut/GL/glut.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/glut/glut32.lib")
		message(ERROR ": GLUT not found")
	endif (NOT EXISTS "${TOOLKITS_FOLDER}/glut/GL/glut.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/glut/glut32.lib")	
	
	if (NOT EXISTS "${TOOLKITS_FOLDER}/glew/GL/glew.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/glew/glew32.lib")
		message(ERROR ": GLEW not found")
	endif (NOT EXISTS "${TOOLKITS_FOLDER}/glew/GL/glew.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/glew/glew32.lib")	

	if (NOT EXISTS "${TOOLKITS_FOLDER}/devil/IL/il.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/devil/DevIL.lib")
		message(ERROR ": DEVIL not found")
	endif (NOT EXISTS "${TOOLKITS_FOLDER}/devil/IL/il.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/devil/DevIL.lib")	
	
	include_directories(
	  	${TOOLKITS_FOLDER}/glut 
		${TOOLKITS_FOLDER}/glew
		${TOOLKITS_FOLDER}/devil
	)
	
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT engine)
	
else (WIN32) 
	find_package(GLUT REQUIRED)
	include_directories(${GLUT_INCLUDE_DIR})
	link_directories(${GLUT_LIBRARY_DIRS})
	add_definitions(${GLUT_DEFINITIONS})
	if (NOT APPLE)
		find_package(GLEW REQUIRED)
		include_directories(${GLEW_INCLUDE_DIRS})
		link_libraries(${GLEW_LIBRARIES})
	endif(NOT APPLE)
	
	find_package(DevIL REQUIRED)
	link_libraries(${IL_LIBRARIES} )
	include_directories(ยง{IL_INCLUDE_DIR})
	
	if(NOT GLUT_FOUND)
	   message(ERROR ": GLUT not found!")
	endif(NOT GLUT_FOUND)
	
endif(WIN32)

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)


include_directories(./engine/headers ./common ./common/headers)
add_definitions(-std=c++11 -Wall)
add_executable(engine engine/engine.cpp engine/tinyxml2.cpp common/hash.cpp engine/figureBuild.cpp engine/xmlHandler.cpp common/matrixOperations.cpp engine/ViewFrustumCulling.cpp)
add_executable(generator generator/generator.cpp common/hash.cpp common/matrixOperations.cpp)

if  (WIN32)
	target_link_libraries(	
		engine 
		${OPENGL_LIBRARIES} 
		${TOOLKITS_FOLDER}/glut/glut32.lib 
		${TOOLKITS_FOLDER}/glew/glew32.lib
		${TOOLKITS_FOLDER}/devil/DevIL.lib
	)
	if (EXISTS "${TOOLKITS_FOLDER}/glut/glut32.dll" AND EXISTS "${TOOLKITS_FOLDER}/glew/glew32.dll" AND EXISTS "${TOOLKITS_FOLDER}/devil/devil.dll")
		file(COPY ${TOOLKITS_FOLDER}/glew/glew32.dll DESTINATION ${CMAKE_BINARY_DIR}/Release)
  		file(COPY ${TOOLKITS_FOLDER}/glut/glut32.dll DESTINATION ${CMAKE_BINARY_DIR}/Release)
  		file(COPY ${TOOLKITS_FOLDER}/devil/devil.dll DESTINATION ${CMAKE_BINARY_DIR}/Release)		
	endif(EXISTS "${TOOLKITS_FOLDER}/glut/glut32.dll" AND EXISTS "${TOOLKITS_FOLDER}/glew/glew32.dll" AND EXISTS "${TOOLKITS_FOLDER}/devil/devil.dll")

else (WIN32)
	target_link_libraries(
		engine 
		${OPENGL_LIBRARIES} 
		${GLUT_LIBRARY} 
	)

endif(WIN32)










