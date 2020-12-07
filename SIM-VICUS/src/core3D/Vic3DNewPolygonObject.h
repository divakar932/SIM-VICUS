/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#ifndef Vic3DNewPolygonObjectH
#define Vic3DNewPolygonObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "Vic3DVertex.h"

#include <VICUS_PlaneGeometry.h>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

class SVPropVertexListWidget;

namespace Vic3D {

class CoordinateSystemObject;
class ShaderProgram;

/*! This object is painted when a new polygon is being drawn.
	It paints a red line from the last confirmed polygon vertex to the current
	coordinate's position (accessing translation coordinates from CoordinateSystemObject directly).
	And if more than 3 vertices have been places already, a transparent plane is painted.

	Uses the same shader as the coordinate system object, but is drawn with blending enabled.
*/
class NewPolygonObject {
public:
	NewPolygonObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current.
		This only initializes the buffers and vertex array object, but does not allocate data.
		This is done in a call to updateBuffers();
	*/
	void create(ShaderProgram * shaderProgram, const CoordinateSystemObject * coordSystemObject);
	void destroy();

	/*! Appends a vertex to the plane geometry and updates the draw buffer. */
	void appendVertex(const IBKMK::Vector3D & p);

	/*! Removes the vertex at the given position.
		idx must be less than number of vertexes!
	*/
	void removeVertex(unsigned int idx);

	/*! Clear current geometry (clears also all buffers). */
	void clear();

	/*! This function is to be called whenever the movable coordinate system changes its (snapped) position.
		The function first compares the point with the currently set point - if no change is recognized, nothing happens.
		If the point was indeed moved, the buffer will be updated and only the last vertex will be updated in the
		GPU memory.
	*/
	void updateLastVertex(const QVector3D & p);

	/*! Populates the color and vertex buffer with data for the "last segment" line and the polygon.
		Resizes vertex and element buffers on GPU memory and copies data from locally stored vertex/element arrays to GPU.

		Basically transfers data in m_vertexBufferData, m_colorBufferData and m_elementBufferData to GPU memory.
	*/
	void updateBuffers();

	/*! Binds the vertex array object and renders the geometry. */
	void render();

	/*! Returns true, if enough vertexes have been collected to complete the geometry.
		This depends on the type of geometry being generated.
	*/
	bool canComplete() const { return m_planeGeometry.isValid(); }

	/*! Can be used to check if object has data to paint. */
	bool hasData() const { return !m_vertexBufferData.empty(); }

	/*! Cached pointer to vertex list widget - for direct communication, when a node has been placed.
		The function appendVertex() relays this call to vertex list widget.
		Pointer is set (by SVPropertyWidget) once widget has been created.
	*/
	SVPropVertexListWidget			*m_vertexListWidget = nullptr;

private:

	/*! Shader program (not owned). */
	ShaderProgram					*m_shaderProgram = nullptr;

	/*! Cached pointer to coordinate system object - used to retrieve current's 3D cursor position (not owned). */
	const CoordinateSystemObject	*m_coordSystemObject = nullptr;

	/*! Stores the current geometry of the painted polygon. */
	VICUS::PlaneGeometry			m_planeGeometry;
	unsigned int					m_firstLineVertex = 0;

	/*! Vertex buffer in CPU memory, holds data of all vertices (coords).
		The last vertex is always the vertex of the current movable coordinate system's location.
		The line will be drawn between the last and the one before last vertex, using array draw command.
	*/
	std::vector<VertexC>			m_vertexBufferData;
	/*! Index buffer on CPU memory (only for the triangle strip). */
	std::vector<GLshort>			m_indexBufferData;

	/*! VertexArrayObject, references the vertex, color and index buffers. */
	QOpenGLVertexArrayObject		m_vao;

	/*! Handle for vertex buffer on GPU memory. */
	QOpenGLBuffer					m_vertexBufferObject;
	/*! Handle for index buffer on GPU memory */
	QOpenGLBuffer					m_indexBufferObject;
};

} // namespace Vic3D

#endif // Vic3DNewPolygonObjectH
