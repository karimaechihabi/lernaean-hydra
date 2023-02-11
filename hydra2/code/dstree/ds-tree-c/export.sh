sh clean.sh
sh autogen.sh
rm -rf *.cache
git archive master | bzip2 > ../isax2plus.tar.bz2