function(get_git_tag output_var)
  #COMMAND git describe --abbrev=0 --tags
  execute_process(
    COMMAND git tag --sort -creatordate
    COMMAND head -n 1
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE _EXIT_CODE
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(_EXIT_CODE EQUAL 0)
    set(${output_var} ${GIT_TAG} PARENT_SCOPE)
  endif()
endfunction(get_git_tag)

function(get_git_commit_hash output_var)
  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE _EXIT_CODE
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(_EXIT_CODE EQUAL 0)
    set(${output_var} ${GIT_COMMIT_HASH} PARENT_SCOPE)
  endif()
endfunction(get_git_commit_hash)
