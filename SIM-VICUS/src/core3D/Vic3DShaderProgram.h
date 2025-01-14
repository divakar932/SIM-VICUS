#ifndef Vic3DShaderProgramH
#define Vic3DShaderProgramH

#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

namespace Vic3D {

/*! A small wrapper class around QOpenGLShaderProgram to encapsulate
	shader code compilation and linking and error handling.

	It is meant to be used with shader programs in files, for example from
	qrc files.

	The embedded shader programm is not destroyed automatically upon destruction.
	You must call destroy() to end the lifetime of the allocated OpenGL resources.
*/
class ShaderProgram {
public:
	ShaderProgram() {}
	ShaderProgram(const QString & vertexShaderFilePath, const QString & fragmentShaderFilePath);

	/*! Creates shader program, compiles and links the programs. */
	void create();
	/*! Destroys OpenGL resources, OpenGL context must be made current before this function is callded! */
	void destroy();

	/*! Access to the native shader program. */
	QOpenGLShaderProgram * shaderProgram() { return m_program; }

	/*! Just forwards to embedded QShaderProgram. */
	void bind();
	/*! Just forwards to embedded QShaderProgram. */
	void release();

	/*! Path to vertex shader program, used in create(). */
	QString		m_vertexShaderFilePath;
	/*! Path to fragment shader program, used in create(). */
	QString		m_fragmentShaderFilePath;


	// Note: Uniform-Handling is pretty simple, probably better to wrap that somehow.

	/*! List of uniform values to be resolved. Values is used in create(). */
	QStringList	m_uniformNames;

	/*! Holds uniform Ids to be used in conjunction with setUniformValue(). */
	QList<int>	m_uniformIDs;

private:
	/*! The wrapped native QOpenGLShaderProgram. */
	QOpenGLShaderProgram	*m_program;
};

} // namespace Vic3D

#endif // Vic3DShaderProgramH
