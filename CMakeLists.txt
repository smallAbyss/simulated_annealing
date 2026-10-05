cmake_minimum_required(VERSION 4.3)

set(PROJECT_NAME_STR sa)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(${PROJECT_NAME_STR})

# Корень старого проекта - задаём var
set(GCG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/GCGen)
# Папка с новым main - задаём var
set(MY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/Simulated_annealing)

# Инклюды: и старые src, и новый My, и GCG целиком
include_directories(
    ${GCG_DIR}/src
    ${GCG_DIR}/sample_src
    ${MY_DIR}
)

# Собираем исходники старого проекта
file(GLOB SRC_FILES
    ${GCG_DIR}/sample_src/*.cpp
    ${GCG_DIR}/sample_src/*.h
    ${GCG_DIR}/sample_src/*.hpp
    ${GCG_DIR}/sample_src/GKLS/*.hpp
    ${GCG_DIR}/sample_src/GKLS/*.cpp
    ${GCG_DIR}/sample_src/Grishagin/*.hpp
    ${GCG_DIR}/sample_src/Grishagin/*.cpp
    ${GCG_DIR}/sample_src/Hill/*.cpp
    ${GCG_DIR}/sample_src/Hill/*.hpp
    ${GCG_DIR}/sample_src/Shekel/*.cpp
    ${GCG_DIR}/src/*.cpp
    ${GCG_DIR}/src/*.h
    ${GCG_DIR}/src/*.hpp
)

# Добавляем твой новый main
file(GLOB MY_SRC 
    ${MY_DIR}/*.cpp 
    ${MY_DIR}/*.h 
    ${MY_DIR}/*.hpp 
    ${MY_DIR}/sa_heads/*.h 
    ${MY_DIR}/sa_heads/*.*)

add_executable(${PROJECT_NAME_STR} ${SRC_FILES} ${MY_SRC})