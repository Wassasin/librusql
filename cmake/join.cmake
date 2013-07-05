#USAGE:
#   SET(somelist a b c)
#   JOIN("${somelist}" ":" output)
#   MESSAGE("${output}") # will output "a:b:c"

function(join VALUES GLUE OUTPUT)
	string(REPLACE ";" "${GLUE}" _TMP_STR "${VALUES}")
	set(${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()
