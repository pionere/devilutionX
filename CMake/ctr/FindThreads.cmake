# Stub out the Threads package on the 3DS.
# 3DS does not have a system threads library but SDL threads are supported.
if(NOT TARGET Threads::Threads)
  add_library(Threads::Threads INTERFACE IMPORTED GLOBAL)
endif()
