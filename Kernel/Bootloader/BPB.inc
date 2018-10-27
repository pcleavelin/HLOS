OSName                       db "16OS    " ;//8 byte.
BytesPerSector_BPB           dw 512
SectorsPerCluster            db 1
ReservedSectors              dw 1
NumberOfFATCopies            db 2
NumberOfPossibleRootEntries  dw 224
Small_NumberOfSectors        dw 2880  ;//18 * 80 * 2
MediaDscriptor               db 0x0f0 ;//Fixed disk 0xF8.
SectorsPerFAT                dw 9
SectorsPerTrack_BPB          dw 18
NumberOfHeads                dw 2
HiddenSectors                dd 0
Large_NumberOfSectors        dd 0

DriveNumber                  db 0    ;0x80 for hard disks
Reserved                     db 0
ExtendedBootSignature        db 0x29 ;Extended boot signature
                                     ;(0x29). This is a signature
                                     ;byte that indicates that the
                                     ;following three fields in the
                                     ;boot sector are present.
VolumeSerialNumber           dd 0x123456
VolumeLabel                  db "16OS DRIVE "   ;11 byte.
FileSystemType               db "FAT12   "      ;8 byte.