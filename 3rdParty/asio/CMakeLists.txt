include(FetchContent_MakeAvailableExcludeFromAll)
include(FetchContent)

FetchContent_Declare(asio
	#URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-18-1.zip
	#URL_HASH MD5=e8eb4612ddaeb0f6e4e865e0d5d94da9
	#URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-18-2.zip
	#URL_HASH MD5=0f297c14ffec04d8ac9bd74f3c8aba36
	URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-31-0.zip
	URL_HASH MD5=f1ade8d4a2b86daf7f5ad90fdbde6691
)
#FetchContent_Declare(asio
#   URL https://github.com/chriskohlhoff/asio/archive/77bcfe775ad63178942c9dd95d93edd10442b80f.zip
#   URL_HASH MD5=bfb3071dff527a6618be8836dc753f0a
#)
FetchContent_MakeAvailableExcludeFromAll(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
