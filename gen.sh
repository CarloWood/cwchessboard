# gen
# Copyright (C) 2006 Carlo Wood
# See LICENSE.WTFPL

if test $# != 1; then
  echo "Usage: $0 CwChessboard.cxx"
  exit 1
fi

echo "#include <iostream>" > gen.cpp
echo "#include <cmath>" >> gen.cpp
echo >> gen.cpp
echo "#define CONST(name, expr) expr; std::cout << \"#define CWCHESSBOARD_CONST_\" << #name << \" \" << name << std::endl;" >> gen.cpp
echo "int main()" >> gen.cpp
echo "{" >> gen.cpp
mawk 'BEGIN                     { inl=0; } \
     /static .*= CONST\(.*;/    { sub(/^[ \t]*/, ""); printf("  %s\n", $0); inl=0; } \
     //                         { if (inl) { sub(/^[ \t]*/, ""); printf("    %s\n", $0); inl=0; } } \
     /static .*= CONST\([^;]*$/ { sub(/^[ \t]*/, ""); printf("  %s\n", $0); inl=1; }' \
     $1 >> gen.cpp
echo "}" >> gen.cpp

