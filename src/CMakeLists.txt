aux_source_directory(. SRC_LIST)

add_library(mprpc ${SRC_LIST})

# muduo库是静态库     .a文件
## 编译动态库时链接静态库会出一些问题
# muduo_base依赖muduo_net      得先链接net
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)

