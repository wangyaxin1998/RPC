aux_source_directory(. SRC_LIST)
add_library(mprpc  ${SRC_LIST})
target_link_libraries(mprpc muduo_net muduo_base  pthread zookeeper_mt)
# muduo_base依赖muduo_net，要先写muduo_net
# 这里动态库链接静态库会出现问题，于是把mprpc也改成静态库