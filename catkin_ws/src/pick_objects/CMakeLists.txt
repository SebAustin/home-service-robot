cmake_minimum_required(VERSION 3.0.2)
project(pick_objects)

add_compile_options(-std=c++14 -Wall -Wextra)

find_package(catkin REQUIRED COMPONENTS
  actionlib
  move_base_msgs
  roscpp
  std_msgs
)

# Exports the Delivery Phase contract and zone loading so add_markers uses the same definitions.
catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}_core
  CATKIN_DEPENDS roscpp std_msgs
)

include_directories(include ${catkin_INCLUDE_DIRS})

add_library(${PROJECT_NAME}_core
  src/zone.cpp
  src/zone_params.cpp
  src/delivery_phase.cpp
  src/delivery_state_machine.cpp
)
target_link_libraries(${PROJECT_NAME}_core ${catkin_LIBRARIES})
add_dependencies(${PROJECT_NAME}_core ${catkin_EXPORTED_TARGETS})

add_executable(pick_objects src/pick_objects_node.cpp)
target_link_libraries(pick_objects ${PROJECT_NAME}_core ${catkin_LIBRARIES})
add_dependencies(pick_objects ${catkin_EXPORTED_TARGETS})

if(CATKIN_ENABLE_TESTING)
  catkin_add_gtest(test_delivery_core test/test_delivery_core.cpp)
  target_link_libraries(test_delivery_core ${PROJECT_NAME}_core)
endif()

install(TARGETS ${PROJECT_NAME}_core pick_objects
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
install(DIRECTORY include/${PROJECT_NAME}/ DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})
