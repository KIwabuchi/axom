# use uberenv to install everything
python uberenv.py --prefix /usr/gapps/asctoolkit/thirdparty_libs/ --spec %gcc@4.9.3
# change group and perms
chgrp -R toolkit /usr/gapps/asctoolkit/thirdparty_libs/
chmod -R g+rwX /usr/gapps/asctoolkit/thirdparty_libs/

