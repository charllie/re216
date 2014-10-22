cmake_minimum_required(VERSION 2.8.3)

project(RE216_JALON03)

source_group("Headers" REGULAR_EXPRESSION ".*\\.h$")

#add here source files you need to compile and link to the client app
SET(SRC_CLIENT
	client.c
	)


add_executable(RE216_JALON03_CLIENT ${SRC_CLIENT})


#add here source files you need to compile and link to the server app
SET(SRC_SERVER
        server.c
        user.c
	)

find_package(Threads)

add_executable(RE216_JALON03_SERVER  ${SRC_SERVER})

target_link_libraries(RE216_JALON03_SERVER ${CMAKE_THREAD_LIBS_INIT})
