set(SRC_LIST userservice.cpp ../friend.pb.cc ../user.pb.cc)

add_executable(provider ${SRC_LIST})

# 链接
target_link_libraries(provider mprpc protobuf zookeeper_mt)
