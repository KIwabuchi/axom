#!/bin/bash
###############################################################################
# use uberenv to build third party libs on chaos 5 using gcc 4.7.1
###############################################################################
#use umask to preserve group perms
source lc_umask.sh
#call uberenv to install our packages
python ../uberenv.py --prefix /usr/gapps/asctoolkit/thirdparty_libs/nightly --spec %gcc@4.7.1

