under development...

If you want to use this library, you should notice:
- apply libcrush-based-on-ceph0.86.diff, then recompile ceph and get a new librados.so
- `./build.sh` to create libcrush.so
- `cd build; make install` to install libcrush.so 
- `-lcrush -lrados`, link libcrush before librados, because librados contains unmodified crush codes

I want to:
- simulate CRUSH algorithm with online data in a live Ceph cluster.
- access crushmap.

How I do:
- adjust Ceph code (based on v0.86), make a new librados.  
- diff: libcrush-based-on-ceph0.86.diff.  

I tried:
- extract crush + client code from kernel code. -- difficult.
- use librados directly. -- no access to crushmap.

