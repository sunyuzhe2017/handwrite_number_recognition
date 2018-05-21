cmake_minimum_required(VERSION 2.8.3)
project(nub_r)

find_package(OpenCV REQUIRED)
find_package(catkin REQUIRED COMPONENTS
  roscpp
)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES nub_r
#  CATKIN_DEPENDS roscpp
#  DEPENDS system_lib
)

include_directories(
 include
  ${OpenCV_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)

# add_executable(${PROJECT_NAME}_node src/nub_r_node.cpp)
add_executable(main src/main.cpp)

## Specify libraries to link a library or executable target against
 target_link_libraries(main
   ${catkin_LIBRARIES}
   ${OpenCV_LIBRARIES}
 )

