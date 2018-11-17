# VMD Library

Library for VMD file which is one of the file type associated to the software
called MikuMikuDance(MMD).

# Note

VMD file contains Shift-JIS encoded (one of the encoding to represent Japanese
character) charactars for names of bones, morphs and other objects. If you want
to print out these strings properly in Linux terminal with UTF-8 encoding, you
can do that by piping output to `iconv -f sjis`.

# Acknowledgements

First of all, I greatly appreciate the creater of MMD, Mr. Higuchi.

I appreciate for the fellows who dedicated to VMD file anaylization.
This library was created by referring to many analysis projects for VMD file.
