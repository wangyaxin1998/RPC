set(SRC_LIST userservice.cc ../user.pb.cc)
add_executable(provider ${SRC_LIST})

#这里依赖了静态库
target_link_libraries(provider mprpc protobuf)