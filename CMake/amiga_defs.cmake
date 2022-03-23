# Disable sanitizers. They're not supported out-of-the-box.
set(ASAN OFF)
set(UBSAN OFF)

# General build options.
set(NONET ON)
set(ADAPTIVE_NETUPDATE OFF)
set(NETENCRYPT OFF)
set(TCPIP OFF)
set(USE_SDL1 ON)
set(SDL1_VIDEO_MODE_BPP 8)

#set(TTF_FONT_NAME \"LiberationSerif-Bold.ttf\")
# Enable exception support as they are used in dvlnet code
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

# Do not warn about unknown attributes, such as [[nodiscard]].
# As this build uses an older compiler, there are lots of them.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes")
