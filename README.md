# xll27

One reason for the popularity of Python is that Guido von Rossom made
it possible to call C from Python. His prior work at CWI on 
[ABC](https://homepages.cwi.nl/~steven/abc/programmers/handbook.html)
did not have this option to make it more perfomant.		

He made sure his C API allowed passing pointers to `double`s without
copying data. Here is the code for calling the Cholesky decompostion from 
[numpy](https://github.com/numpy/numpy/blob/4ff27bbc1379eddd8e9a75f5d8ae35bf3388a570/numpy/linalg/umath_linalg.cpp#L2043)

Here is how to call it using the xll library

One reason for the unpopularity of the
[Excel C SDK](https://learn.microsoft.com/en-us/office/client-developer/excel/developing-excel-xlls)
is that it is also a C API that is non-trivial to call.