cmake_minimum_required(VERSION 3.0.2)
project(add_markers)

add_compile_options(-std=c++14 -Wall -Wextra)

find_package(catkin REQUIRED COMPONENTS
  pick_objects
  roscpp
  std_msgs
  visualization_msgs
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}_policy
  CATKIN_DEPENDS pick_objects roscpp std_msgs visualization_msgs
)

include_directories(include ${catkin_INCLUDE_DIRS})

add_library(${PROJECT_NAME}_policy src/marker_policy.cpp)
target_link_libraries(${PROJECT_NAME}_policy ${catkin_LIBRARIES})
add_dependencies(${PROJECT_NAME}_policy ${catkin_EXPORTED_TARGETS})

add_executable(add_markers src/add_markers_node.cpp)
target_link_libraries(add_markers ${PROJECT_NAME}_policy ${catkin_LIBRARIES})
add_dependencies(add_markers ${catkin_EXPORTED_TARGETS})

if(CATKIN_ENABLE_TESTING)
  catkin_add_gtest(test_marker_policy test/test_marker_policy.cpp)
  target_link_libraries(test_marker_policy ${PROJECT_NAME}_policy ${catkin_LIBRARIES})
endif()

install(TARGETS ${PROJECT_NAME}_policy add_markers
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
