#! /bin/sh

srcdir="$(dirname "$0")"
grep -B 1000 'RULE DECLARATION START' "${srcdir}/PgnGrammar.h.in" > PgnGrammar.h
awk 'BEGIN { b=0 }
    /definition\(PgnGrammar const& self\)/ { b=1 }
    /DEBUG MACROS START/ { b=0 }
    /^[[:space:]]*=[[:space:]]*$$/ { if (b)
      if (line == "blank" || line == "white_space" || line == "eol" || line == "escaped_data")
        printf("      no_tree_rule_t %s;\n", line);
      else
        printf("      rule_t %s;\n", line); }
    { if (b == 1) { sub(/[[:space:]]*/, ""); sub(/[[:space:]]*\/.*/, ""); line=$$0; } }' "${srcdir}/PgnGrammar.h.in" >> PgnGrammar.h
grep -A 1000 'RULE DECLARATION END' "${srcdir}/PgnGrammar.h.in" | grep -B 1000 'DEBUG MACROS START' >> PgnGrammar.h
awk 'BEGIN { b=0 }
    /definition\(PgnGrammar const& self\)/ { b=1 }
    /DEBUG MACROS START/ { b=0 }
    /^[[:space:]]*=[[:space:]]*$$/ { if (b == 1) printf("        BOOST_SPIRIT_DEBUG_TRACE_RULE(%s, TRACE_PGN_GRAMMAR%s);\n", a[1], a[2]) }
    { if (b == 1) { sub(/^[[:space:]]*/, "");
      split($$0, a, /[[:space:]\/]*/);
      if (length(a[2]) > 0) a[2] = sprintf(" & %s", a[2]); } }' "${srcdir}/PgnGrammar.h.in" >> PgnGrammar.h
grep -A 1000 'DEBUG MACROS END' "${srcdir}/PgnGrammar.h.in" >> PgnGrammar.h
