add_library(DeliriX
	src/api.cpp
	src/zip.cpp
	src/txt.cpp
	src/xml.cpp
	src/odt.cpp
	src/docx.cpp
	src/fb2.cpp
	src/dump-as-json.cpp
	src/dump-as-tags.cpp
	src/load-as-json.cpp
	src/load-as-tags.cpp)

add_subdirectory(tests)
