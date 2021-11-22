set(SRC_LIST calluserservice.cc ../user.pb.cc)
add_executable(consumer ${SRC_LIST})

#这里依赖了静态库
target_link_libraries(consumer mprpc protobuf)