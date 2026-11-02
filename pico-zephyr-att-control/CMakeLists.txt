cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pico_zephyr_att_control)
target_sources(app PRIVATE src/main.c src/attenuator.c src/att_codec.c src/http.c src/network/network.c src/network/tcp_console.c)
