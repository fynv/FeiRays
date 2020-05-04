from .Native import native

class Vec3:
    def __init__(self, t):
        self.m_cptr = native.n_vec3_create(t[0],t[1],t[2])
    def __del__(self):
        native.n_vec3_destroy(self.m_cptr)

class Transform:
    def __init__ (self):
        self.m_cptr = native.n_transform_create()

    def __del__ (self):
        native.n_transform_destroy(self.m_cptr)

    def translate(self, x, y, z):
        v = Vec3((x,y,z))
        native.n_transform_translate(self.m_cptr, v.m_cptr)

    def rotate(self, angle, axis):
        v_axis = Vec3(axis)
        native.n_transform_rotate(self.m_cptr, angle, v_axis.m_cptr)

    def scale(self, x, y, z):
        v = Vec3((x,y,z))
        native.n_transform_scale(self.m_cptr, v.m_cptr)
