#include "graphics/geometry.h"
#include "core/math_utils.h"
#include "core/profiler.h"
#include "graphics/gl_ext.h"
#include "graphics/shader.h"

namespace Lumix
{


void VertexDef::parse(const char* data, int size)
{
	m_vertex_size = 0;
	int index = 0;
	for(int i = 0; i < size; ++i)
	{
		ASSERT(index < 15);
		switch(data[i])
		{
			case 'f':
				++i;
				if (data[i] == '4')
				{
					m_attributes[index] = VertexAttributeDef::FLOAT4;
					m_vertex_size += 4 * sizeof(float);
				}
				else if (data[i] == '2')
				{
					m_attributes[index] = VertexAttributeDef::FLOAT2;
					m_vertex_size += 2 * sizeof(float);
				}
				else
				{
					ASSERT(false);
				}
				break;
			case 'i':
				++i;
				if (data[i] == '4')
				{
					m_attributes[index] = VertexAttributeDef::INT4;
					m_vertex_size += 4 * sizeof(int);
				}
				else if (data[i] == '1')
				{
					m_attributes[index] = VertexAttributeDef::INT1;
					m_vertex_size += sizeof(int);
				}
				else
				{
					ASSERT(false);
				}
				break;
			case 'p':
				m_attributes[index] = VertexAttributeDef::POSITION;
				m_vertex_size += 3 * sizeof(float);
				break;
			case 'n':
				m_attributes[index] = VertexAttributeDef::NORMAL;
				m_vertex_size += 3 * sizeof(float);
				break;
			case 't':
				m_attributes[index] = VertexAttributeDef::TEXTURE_COORDS;
				m_vertex_size += 2 * sizeof(float);
				break;
			default:
				ASSERT(false);
				break;
		}
		++index;
	}
	m_attributes[index] = VertexAttributeDef::NONE;
	m_attribute_count = index;
}


int VertexDef::getPositionOffset() const
{
	int offset = 0;
	for(int i = 0; i < m_attribute_count; ++i)
	{
		switch(m_attributes[i])
		{
			case VertexAttributeDef::FLOAT2:
				offset += 2 * sizeof(float);
				break;
			case VertexAttributeDef::FLOAT4:
				offset += 4 * sizeof(float);
				break;
			case VertexAttributeDef::INT4:
				offset += 4 * sizeof(int);
				break;
			case VertexAttributeDef::INT1:
				offset += sizeof(int);
				break;
			case VertexAttributeDef::POSITION:
				return offset;
				break;
			case VertexAttributeDef::NORMAL:
				offset += 3 * sizeof(float);
				break;
			case VertexAttributeDef::TEXTURE_COORDS:
				offset += 2 * sizeof(float);
				break;
			default:
				ASSERT(false);
				break;
		}
	}
	return -1;
}


void VertexDef::begin(Shader& shader) const 
{
	PROFILE_FUNCTION();
	int offset = 0;
	int shader_attrib_idx = 0;
	int attribute_count = Math::minValue(m_attribute_count, shader.getAttributeCount());
	for(int i = 0; i < attribute_count; ++i)
	{
		GLint attrib_id = shader.getAttribId(shader_attrib_idx);
		switch(m_attributes[i])
		{
			case VertexAttributeDef::POSITION:
				glEnableVertexAttribArray(attrib_id);
				glVertexAttribPointer(attrib_id, 3, GL_FLOAT, GL_FALSE, m_vertex_size, (GLvoid*)offset);
				offset += sizeof(GLfloat) * 3;
				++shader_attrib_idx;
				break;
			case VertexAttributeDef::NORMAL:
				glEnableVertexAttribArray(attrib_id);
				glVertexAttribPointer(attrib_id, 3, GL_FLOAT, GL_FALSE, m_vertex_size, (GLvoid*)offset);
				offset += sizeof(GLfloat) * 3;
				++shader_attrib_idx;
				break;
			case VertexAttributeDef::TEXTURE_COORDS:
				glEnableVertexAttribArray(attrib_id);
				glVertexAttribPointer(attrib_id, 2, GL_FLOAT, GL_FALSE, m_vertex_size, (GLvoid*)offset);
				offset += sizeof(GLfloat) * 2;
				++shader_attrib_idx;
				break;
			case VertexAttributeDef::FLOAT2:
				glEnableVertexAttribArray(attrib_id);
				glVertexAttribPointer(attrib_id, 2, GL_FLOAT, GL_FALSE, m_vertex_size, (GLvoid*)offset);
				offset += sizeof(GLfloat) * 2;
				++shader_attrib_idx;
				break;
			case VertexAttributeDef::FLOAT4:
				glEnableVertexAttribArray(attrib_id);
				glVertexAttribPointer(attrib_id, 4, GL_FLOAT, GL_FALSE, m_vertex_size, (GLvoid*)offset);
				offset += sizeof(GLfloat) * 4;
				++shader_attrib_idx;
				break;
			case VertexAttributeDef::INT4:
				glEnableVertexAttribArray(attrib_id);
				glVertexAttribPointer(attrib_id, 4, GL_INT, GL_FALSE, m_vertex_size, (GLvoid*)offset);
				offset += sizeof(GLint) * 4;
				++shader_attrib_idx;
				break;
			case VertexAttributeDef::INT1:
				glEnableVertexAttribArray(attrib_id);
				glVertexAttribPointer(attrib_id, 1, GL_INT, GL_FALSE, m_vertex_size, (GLvoid*)offset);
				offset += sizeof(GLint) * 1;
				++shader_attrib_idx;
				break;
			default:
				ASSERT(false);
				break;
		}
	}
	
}


void VertexDef::end(Shader& shader) const
{
	PROFILE_FUNCTION();
	int shader_attrib_idx = 0;
	for(int i = 0; i < m_attribute_count; ++i)
	{
		switch(m_attributes[i])
		{
			case VertexAttributeDef::POSITION:
			case VertexAttributeDef::NORMAL:
			case VertexAttributeDef::TEXTURE_COORDS:
			case VertexAttributeDef::INT1:
			case VertexAttributeDef::INT4:
			case VertexAttributeDef::FLOAT4:
			case VertexAttributeDef::FLOAT2:
				glDisableVertexAttribArray(shader.getAttribId(shader_attrib_idx));
				++shader_attrib_idx;
				break;
			default:
				ASSERT(false);
				break;
		}
	}
	
}


AABB Geometry::getAABB() const
{
	Vec3 min = m_vertices[0];
	Vec3 max = m_vertices[0];
	for(int i = 1, c = m_vertices.size(); i < c; ++i)
	{
		min.x = Math::minValue(min.x, m_vertices[i].x);
		min.y = Math::minValue(min.y, m_vertices[i].y);
		min.z = Math::minValue(min.z, m_vertices[i].z);

		max.x = Math::maxValue(max.x, m_vertices[i].x);
		max.y = Math::maxValue(max.y, m_vertices[i].y);
		max.z = Math::maxValue(max.z, m_vertices[i].z);
	}
	return AABB(min, max);
}


float Geometry::getBoundingRadius() const
{
	float d = 0;
	for(int i = 0, c = m_vertices.size(); i < c; ++i)
	{
		float l = m_vertices[i].squaredLength();
		if(l > d)
		{
			d = l;
		}
	}
	return sqrtf(d);
}


Geometry::Geometry()
{
	glGenBuffers(1, &m_id);
	glGenBuffers(1, &m_indices_id);
}


Geometry::~Geometry()
{
	glDeleteBuffers(1, &m_id);
	glDeleteBuffers(1, &m_indices_id);
}


void Geometry::copy(const Geometry& source, int times, VertexCallback& vertex_callback, IndexCallback& index_callback)
{
	ASSERT(!source.m_indices.empty());
	m_vertex_definition = source.m_vertex_definition;

	glBindBuffer(GL_ARRAY_BUFFER, source.m_id);
	uint8_t* data = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	Array<uint8_t> data_copy;
	int vertex_size = m_vertex_definition.getVertexSize();
	int one_size = vertex_size * source.getVertices().size();
	data_copy.resize(one_size * times);
	for (int i = 0; i < times; ++i)
	{
		memcpy(&data_copy[i * one_size], data, one_size);
	}
	vertex_callback.invoke(data_copy);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, source.m_indices_id);
	data = (uint8_t*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
	Array<int> indices_data_copy;
	int indices_count = source.getIndices().size();
	indices_data_copy.resize(indices_count * times);
	for (int i = 0; i < times; ++i)
	{
		memcpy(&indices_data_copy[i * indices_count], data, sizeof(int) * indices_count);
	}
	index_callback.invoke(indices_data_copy);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
	glBufferData(GL_ARRAY_BUFFER, data_copy.size() * sizeof(data_copy[0]), &data_copy[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_data_copy.size() * sizeof(indices_data_copy[0]), &indices_data_copy[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void Geometry::copy(const uint8_t* data, int size, const Array<int32_t>& indices, VertexDef vertex_definition)
{
	m_vertex_definition = vertex_definition;
	int vertex_size = m_vertex_definition.getVertexSize();
	m_vertices.resize(size / vertex_size);
	int pos_offset = m_vertex_definition.getPositionOffset();
	for (int i = 0, c = m_vertices.size(); i < c; ++i)
	{
		m_vertices[i] = *reinterpret_cast<const Vec3*>(data + vertex_size * i + pos_offset);
	}
	m_indices.resize(indices.size());
	for (int i = 0, c = m_indices.size(); i < c; ++i)
	{
		m_indices[i] = indices[i];
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


} // ~namespace Lumix
