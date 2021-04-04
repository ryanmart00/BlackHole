#ifndef RGB_HPP_INCLUDED
#define RGB_HPP_INCLUDED
#include <glad/glad.h>

struct RGB
{
    GLubyte r, g, b;
    RGB(GLubyte x, GLubyte y, GLubyte z) : r{x}, g{y}, b{z}
    {
    }

};


#endif
