#ifndef Vic3DWireFrameObjectH
#define Vic3DWireFrameObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "Vic3DVertex.h"
#include "Vic3DTransform3D.h"

#include <set>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

namespace VICUS {
	class Object;
}

namespace Vic3D {

class ShaderProgram;

/*! A container for geometry to be rendered with either triangle strips or triangles.
	This object is used multiple times and is not associated with any particular
	part of the scene or building. Hence, it is populated with data, externally.

	Rendering is done in a dual pass - first the geometry is rendered in polygon-mode, and
	afterwards in fill-mode. When in polygon mode, the shader is configured to brighten/darken
	the colors a bit.

	The object is ment to be used for movable, selected geometry, and uses the shaders:
	- VertexNormalColorWithTransform.vert
	- phong_lighting.frag (in fill mode)
	- phong_lighting_wireframe.frag (in polygon mode)

	The switch of the shader programs is done in the render function.
*/
class WireFrameObject {
public:
	WireFrameObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current.
		This only initializes the buffers and vertex array object, but does not allocate data.
		This is done in a call to updateBuffers();
	*/
	void create(ShaderProgram * shaderProgram);
	void destroy();

	/*! Resizes vertex and element buffers on GPU memory and copies data from locally
		stored vertex/element arrays to GPU.
		This might be a lengthy operation, so call this as infrequently as possible.

		Basically transfers data in m_vertexBufferData, m_colorBufferData and
		m_elementBufferData to GPU memory.
		Calls the function updateColorBuffer() internally to update the color buffer.
	*/
	void updateBuffers();

	/*! Binds the vertex array object and renders the geometry. */
	void render();

	/*! Processed project data and updates the m_selectedObjects vector based
		on the selection property of the objects.
		\param selectedObjects Here the pointers of selected objects will be stored.
		\param takeInvisible If true, only invisible objects with selection status are taken. If false, only visible
							 selected objects will be taken.
	*/
	static void updateSelectedObjectsFromProject(std::set<const VICUS::Object *> & selectedObjects, bool takeInvisible=false);

	/*! The transformation from model coordinates to (current) world coordinates. */
	Transform3D					m_transform;

	/*! This set caches the list of current selected objects.
		This set is processed in updateBuffers() to fill the coordinate buffers.
	*/
	std::set<const VICUS::Object*>		m_selectedObjects;

	ShaderProgram						*m_shaderProgram = nullptr;

	/*! Vertex buffer in CPU memory, holds data of all vertices (coords and normals). */
	std::vector<VertexC>				m_vertexBufferData;
	/*! Index buffer on CPU memory. */
	std::vector<GLuint>					m_indexBufferData;

	/*! Maps unique surface/node ID to vertex start index in m_vertexBufferData. */
	std::map<unsigned int, unsigned int>	m_vertexStartMap;

	/*! VertexArrayObject, references the vertex, color and index buffers. */
	QOpenGLVertexArrayObject			m_vao;

	/*! Handle for vertex buffer on GPU memory. */
	QOpenGLBuffer						m_vertexBufferObject;
	/*! Handle for index buffer on GPU memory */
	QOpenGLBuffer						m_indexBufferObject;
};

} // namespace Vic3D

#endif // Vic3DWireFrameObjectH
