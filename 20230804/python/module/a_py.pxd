from libcpp.string cimport string

cdef extern from "A.h":
    cdef cppclass A:
        A() except +
        A(int) except +
        string getMe()
