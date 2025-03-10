set(SRC_LIST calluserservice.cpp ../user.pb.cc)

add_executable(consumer ${SRC_LIST})

# 链接
target_link_libraries(consumer mprpc protobuf)
