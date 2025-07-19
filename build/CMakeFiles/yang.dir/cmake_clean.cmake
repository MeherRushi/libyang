file(REMOVE_RECURSE
  "libyang.pdb"
  "libyang.so"
  "libyang.so.3"
  "libyang.so.3.9.1"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/yang.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
