cmake_minimum_required(VERSION 4.3)

set(PROJECT_NAME_STR sa)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(${PROJECT_NAME_STR})

set(LIBS ${CMAKE_CURRENT_SOURCE_DIR}/src/libs)
set(cppPlot ${CMAKE_CURRENT_SOURCE_DIR}/src/app/cppPlot)
set(cppSA  ${CMAKE_CURRENT_SOURCE_DIR}/src/app/cppSA)
set(THIRD ${CMAKE_CURRENT_SOURCE_DIR}/third-party)


include_directories(
    ${LIBS}/GCGen/src
    ${LIBS}/GCGen/sample_src
    ${LIBS}/Logger
    ${cppSA}/
    ${cppSA}/sa
    ${THIRD}/
)


file(GLOB GCGen_SRC
    ${LIBS}/GCGen/sample_src/*.cpp
    ${LIBS}/GCGen/sample_src/*.h
    ${LIBS}/GCGen/sample_src/*.hpp
    ${LIBS}/GCGen/sample_src/GKLS/*.hpp
    ${LIBS}/GCGen/sample_src/GKLS/*.cpp
    ${LIBS}/GCGen/sample_src/Grishagin/*.hpp
    ${LIBS}/GCGen/sample_src/Grishagin/*.cpp
    ${LIBS}/GCGen/sample_src/Hill/*.cpp
    ${LIBS}/GCGen/sample_sr
    ${LIBS}/GCGen/src/*.h
    ${LIBS}/GCGen/src/*.hc/Hill/*.hpp
    ${LIBS}/GCGen/sample_src/Shekel/*.cpp
    ${LIBS}/GCGen/src/*.cpp
    ${LIBS}/GCGen/src/*.h
    ${LIBS}/GCGen/src/*.hpp
)


file(GLOB SA_SRC 
    ${cppSA}/*.cpp 
    ${cppSA}/*.h 
    ${cppSA}/*.hpp 
    ${cppSA}/sa/*.h 
    ${cppSA}/sa/*.*
)

file(GLOB LOGGER_SRC
    ${LIBS}/Logger
)

file(GLOB PLOT_SRC
    ${cppPlot}/*
)

file(GLOB THIRD_PARTY_SRC
    ${THIRD}/*
)


find_package(OpenMP REQUIRED)
add_executable(${PROJECT_NAME_STR} ${GCGen_SRC} ${SA_SRC} ${LOGGER_SRC} ${THIRD_PARTY_SRC})
target_link_libraries(${PROJECT_NAME_STR} PRIVATE OpenMP::OpenMP_CXX)


add_executable(grid ${GCGen_SRC} ${PLOT_SRC})