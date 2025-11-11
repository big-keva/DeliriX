function( add_sample_as_cpp output source vaname vasize )

  get_filename_component( OutDir ${output} DIRECTORY )

  add_custom_command(OUTPUT ${output}
        COMMAND echo "unsigned char" ${vaname}[] = {              > ${output}
        COMMAND xxd -i < ${source}                               >> ${output}
        COMMAND echo "}\;"                                       >> ${output}
        COMMAND echo "unsigned" ${vasize} = `stat --printf="%s" ${source}`"\;" >> ${output}
        DEPENDS ${OutDir} ${source} )

  if ( NOT TARGET ${OutDir} )
    add_custom_target( ${OutDir} ALL
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OutDir})
  endif()

endfunction()
