project(mh_stuff)

add_library(mh_stuff INTERFACE)
add_library(mh::stuff ALIAS mh_stuff)

target_include_directories(mh_stuff INTERFACE cpp/include)
