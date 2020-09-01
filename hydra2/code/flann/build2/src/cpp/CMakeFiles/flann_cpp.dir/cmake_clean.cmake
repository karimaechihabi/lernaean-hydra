file(REMOVE_RECURSE
  "../../lib/libflann_cpp.pdb"
  "../../lib/libflann_cpp.so"
  "../../lib/libflann_cpp.so.1.9.1"
  "../../lib/libflann_cpp.so.1.9"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/flann_cpp.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
