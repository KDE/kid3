# Generate Qt resource file for qm translation files.
# Usage: cmake -P gentranslationsqrc.cmake OUTFILE QMDIR PREFIX
# CMAKE_ARGV0, CMAKE_ARGV1, CMAKE_ARGV2 are
# /path/to/cmake, -P, /path/to/gentranslationsqrc.cmake
set(_outFile ${CMAKE_ARGV3})
get_filename_component(_qmDir ${CMAKE_ARGV4} REALPATH)
set(_prefix ${CMAKE_ARGV5})
file(GLOB _qmFiles "${_qmDir}/*.qm")
file(WRITE ${_outFile}
     "<!DOCTYPE RCC><RCC version=\"1.0\">\n  <qresource prefix=\"${_prefix}\">\n")
foreach(_qmPath ${_qmFiles})
  get_filename_component(_qmFile ${_qmPath} NAME)
  file(APPEND ${_outFile} "    <file alias=\"${_qmFile}\">${_qmPath}</file>\n")
endforeach()
file(APPEND ${_outFile} "  </qresource>\n</RCC>\n")
