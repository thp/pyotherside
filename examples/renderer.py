import numpy

from OpenGL.GL import *
from OpenGL.GL.shaders import compileShader, compileProgram

VERTEX_SHADER = """#version 120
attribute vec4 vertices;
varying vec2 coords;

void main() {
    gl_Position = vertices;
    coords = vertices.xy;
}
"""

FRAGMENT_SHADER = """#version 120
uniform float t;
varying vec2 coords;
void main() {
    float i = 1. - (pow(abs(coords.x), 4.) + pow(abs(coords.y), 4.));
    i = smoothstep(t - 0.8, t + 0.8, i);
    i = floor(i * 20.) / 20.;
    gl_FragColor = vec4(coords * .5 + .5, i, i);
}
"""

class Renderer(object):

    def __init__(self):
        self.t = 0.0
        self.values = numpy.array([
            -1.0, -1.0,
            1.0, -1.0,
            -1.0, 1.0,
            1.0, 1.0
        ], dtype=numpy.float32)

    def set_t(self, t):
        self.t = t

    def init(self):
        self.vertexbuffer = glGenBuffers(1)
        vertex_shader = compileShader(VERTEX_SHADER, GL_VERTEX_SHADER)
        fragment_shader = compileShader(FRAGMENT_SHADER, GL_FRAGMENT_SHADER)
        self.program = compileProgram(vertex_shader, fragment_shader)
        self.vertices_attr = glGetAttribLocation(self.program, b'vertices')
        self.t_attr = glGetUniformLocation(self.program, b't')

    def reshape(self, x, y, width, height):
        glViewport(x, y, width, height)

    def render(self):
        glUseProgram(self.program)
        try:
            glDisable(GL_DEPTH_TEST)
            glClearColor(0, 0, 0, 1)
            glClear(GL_COLOR_BUFFER_BIT)
            glEnable(GL_BLEND)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE)

            glBindBuffer(GL_ARRAY_BUFFER, self.vertexbuffer)
            glEnableVertexAttribArray(self.vertices_attr)
            glBufferData(GL_ARRAY_BUFFER, self.values, GL_STATIC_DRAW)
            glVertexAttribPointer(self.vertices_attr, 2, GL_FLOAT, GL_FALSE, 0, None)
            glUniform1f(self.t_attr, self.t)

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4)
        finally:
            glDisableVertexAttribArray(0)
            glBindBuffer(GL_ARRAY_BUFFER, 0)
            glUseProgram(0)

    def cleanup(self):
        glDeleteProgram(self.program)
        glDeleteBuffers(1, [self.vertexbuffer])
