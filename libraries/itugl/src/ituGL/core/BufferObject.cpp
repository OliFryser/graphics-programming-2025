#include <ituGL/core/BufferObject.h>

// Create the object initially null, get object handle and generate 1 buffer
BufferObject::BufferObject() : Object(NullHandle)
{
    Handle& handle = GetHandle();
    glGenBuffers(1, &handle);
}

// Get object handle and delete 1 buffer
BufferObject::~BufferObject()
{
    Handle& handle = GetHandle();
    glDeleteBuffers(1, &handle);
}

// Bind the buffer handle to the specific target
void BufferObject::Bind(Target target) const
{
    Handle handle = GetHandle();
    glBindBuffer(GL_ARRAY_BUFFER, handle);
}

// Bind the null handle to the specific target
void BufferObject::Unbind(Target target)
{
    Handle handle = NullHandle;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Get buffer Target and allocate buffer data
void BufferObject::AllocateData(size_t size, Usage usage)
{
    Target target = GetTarget();
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, usage);
}

// Get buffer Target and allocate buffer data
void BufferObject::AllocateData(std::span<const std::byte> data, Usage usage)
{
    Target target = GetTarget();
    glBufferData(GL_ARRAY_BUFFER, data.size(), data.data(), usage);
}

// Get buffer Target and set buffer subdata
void BufferObject::UpdateData(std::span<const std::byte> data, size_t offset)
{
    // (todo) 00.5: Update buffer subdata
}
