# gen
# Copyright (C) 2006 Carlo Wood
# See LICENSE.WTFPL

if test $# != 1; then
  echo "Usage: $0 CwChessboard.cc"
  exit 1
fi

echo "#include <iostream>" > gen.cc
echo "#include <cmath>" >> gen.cc
echo >> gen.cc
echo "#define CONST(name, expr) expr; std::cout << \"#define CWCHESSBOARD_CONST_\" << #name << \" \" << name << std::endl;" >> gen.cc
echo "int main()" >> gen.cc
echo "{" >> gen.cc
mawk 'BEGIN                     { inl=0; } \
     /static .*= CONST\(.*;/    { sub(/^[ \t]*/, ""); printf("  %s\n", $0); inl=0; } \
     //                         { if (inl) { sub(/^[ \t]*/, ""); printf("    %s\n", $0); inl=0; } } \
     /static .*= CONST\([^;]*$/ { sub(/^[ \t]*/, ""); printf("  %s\n", $0); inl=1; }' \
     $1 >> gen.cc
echo "}" >> gen.cc

