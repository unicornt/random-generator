project(mycsmith)

set(CMAKE_CXX_FLAGS "-g")

set(sourceCode
    Random.h
    Random.cpp
    StructGenerator.h
    StructGenerator.cpp
    common/common.h
    common/common.cpp
    common/type.h
    common/type.cpp
    common/header.h
    common/json.hpp
)


add_executable(
    TryFunc
    TryFunc.cpp
    ${sourceCode}
)