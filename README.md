under development...

I want to:

- simulate CRUSH algorithm with online data in a live Ceph cluster.
- access crushmap.

How I do:
- adjust Ceph code (based on v0.86), make a new librados.  
- diff: libcrush-based-on-ceph0.86.diff.  

I tried:

- extract crush + client code from kernel code. -- difficult.
- use librados directly. -- no access to crushmap.

