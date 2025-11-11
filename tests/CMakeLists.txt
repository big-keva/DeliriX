set(PROJECT_NAME DeliriX-test)
project(${PROJECT_NAME})

set(SourceDir ${${PROJECT_NAME}_SOURCE_DIR})

include(samples.cmake)

link_libraries(DeliriX moonycode mtc tinyxml2 minizip z)

add_sample_as_cpp(samples/zipzip.cpp ${SourceDir}/samples/zip.zip
	sample_zipzip_buf
	sample_zipzip_len )

add_sample_as_cpp(samples/odtzip.cpp ${SourceDir}/samples/keva.odt
	sample_odtzip_buf
	sample_odtzip_len )

add_sample_as_cpp(samples/odtDeliriX.cpp ${SourceDir}/samples/DeliriX.odt
	sample_odtDeliriX_buf
	sample_odtDeliriX_len )

add_sample_as_cpp(samples/docxDeliriX.cpp ${SourceDir}/samples/DeliriX.docx
	sample_docxDeliriX_buf
	sample_docxDeliriX_len )

add_sample_as_cpp(samples/fb2Panov.cpp ${SourceDir}/samples/panov.fb2
	sample_fb2Panov_buf
	sample_fb2Panov_len )

add_executable(test-Delirix
	test-text.cpp
	test-base.cpp
	test-odt.cpp
	test-docx.cpp
	test-fb2.cpp
	test-main.cpp

	samples/zipzip.cpp
	samples/odtzip.cpp
	samples/odtDeliriX.cpp
	samples/docxDeliriX.cpp
	samples/fb2Panov.cpp)
