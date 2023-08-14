# cython: language_level=3
# distutils: language=c++

cimport a_py as apy

cdef class A:
    cdef apy.A *thisa
    
    def __cinit__(self, int val=0):
        self.thisa = new apy.A(val)
        
    def __dealloc___(self):
        del self.thisa
    def get_me(self):
        return self.thisa.getMe()


print("Imported")