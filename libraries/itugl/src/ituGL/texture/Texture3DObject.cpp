#include <ituGL/texture/Texture3DObject.h>

#include <cassert>

Texture3DObject::Texture3DObject()
{
}

template <>
void Texture3DObject::SetImage<std::byte>(GLint level, GLsizei width, GLsizei height, GLsizei depth, Format format, InternalFormat internalFormat, std::span<const std::byte> data, Data::Type type)
{
    assert(IsBound());
    assert(data.empty() || type != Data::Type::None);
    assert(IsValidFormat(format, internalFormat));
    assert(data.empty() || data.size_bytes() == width * height * depth * GetDataComponentCount(internalFormat) * Data::GetTypeSize(type));
    glTexImage3D(GetTarget(), level, internalFormat, width, height, depth, 0, format, type == Data::Type::None ? GL_BYTE : static_cast<GLenum>(type), data.data());
}

void Texture3DObject::SetImage(GLint level, GLsizei width, GLsizei depth, GLsizei height, Format format, InternalFormat internalFormat)
{
    SetImage<float>(level, width, height, depth, format, internalFormat, std::span<float>());
}
