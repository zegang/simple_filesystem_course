#/bin/evn bash
sudo umount -f mymnt
mkdir mymnt
make
./fuse_teninodesfs mymnt
stat -f mymnt
echo
echo "------To mkdir mymnt/d1"
mkdir mymnt/d1
ls -il mymnt
echo
echo "------To check FS stat again"
stat -f mymnt
