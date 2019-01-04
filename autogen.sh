#!/bin/bash - 
#===============================================================================
#
#          FILE: autogen.sh
# 
#         USAGE: ./autogen.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: Dallen Wilson (), dwjwilson@lavabit.com
#  ORGANIZATION: 
#       CREATED: 2019-01-03 22:35
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

autoreconf --install
automake --add-missing

