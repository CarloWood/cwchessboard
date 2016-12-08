#! /bin/sh

# This file is derived from a template file distributed
# with cwautomacros (http://cwautomacros.berlios.de/).

# Copyright (C) 2006 Carlo Wood

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

# Clueless user check.
if test ! -d CVS -a ! -d .svn -a -f configure; then
  echo "You only need to run './autogen.sh' when you checked out this project using SVN."
  echo "Just run ./configure [--help]."
  echo "If you insist on running it, then first remove the 'configure' script."
  exit 0
fi

# Installation prefix. This has to match what was used while installing cwautomacros of course.
PREFIX=${CWAUTOMACROSPREFIX-/usr}

if test ! -f $PREFIX/share/cwautomacros/scripts/autogen.sh; then
  echo "$0: $PREFIX/share/cwautomacros/scripts/autogen.sh: No such file or directory"
  echo "$0: This project needs 'cwautomacros'. See http://cwautomacros.berlios.de/"
  exit 126
fi

exec $PREFIX/share/cwautomacros/scripts/autogen.sh
