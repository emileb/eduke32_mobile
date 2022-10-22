Compiler options used while creating these binaries

Windows:	make AWOL=1
Linux:		make AWOL=2 CUSTOMOPT="-no-pie"

I'm not entirely certain what the additional flag here is used for with the Linux build, but it may have something to do with the alignment of arrays which may lead to save games causing crashes.

=============================================

Any time that a new/updated group file is created, after updating the awol.grpinfo file with the new file size and CRC32 checksum values, these two properties also need to be updated on lines 60 and 61 of ``source/duke3d/src/grpscan.h`` and then recompiled.

=============================================

Current builds are a derivative form of eduke32_20210829-9597-bb01a1394 which has been modified exclusively for producing A.W.O.L. binaries.