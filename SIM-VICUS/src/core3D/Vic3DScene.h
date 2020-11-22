#ifndef VIC3DSCENE_H
#define VIC3DSCENE_H

#include <vector>

#include <QRect>
#include <QVector3D>

#include "Vic3DCamera.h"
#include "Vic3DGridObject.h"
#include "Vic3DOpaqueGeometryObject.h"
#include "Vic3DOrbitControllerObject.h"
#include "Vic3DCoordinateSystemObject.h"

class ModificationInfo;

namespace VICUS {
	class Surface;
}

namespace Vic3D {

class ShaderProgram;
class KeyboardMouseHandler;

/*! Encapsulates all data for drawing a part of the final picture, including the viewport where the
	scene is rendered in.
*/
class Vic3DScene {
public:
	void create(std::vector<ShaderProgram> & shaderPrograms);

	/*! Triggered when SVProjectHandler::modified() is emitted. */
	void onModified( int modificationType, ModificationInfo * data );

	void destroy();

	void resize(int width, int height, qreal retinaScale);

	/*! Returns current size. */
	QSize currentSize() const { return m_viewPort.size(); }

	/*! Compines camera matrix and project matrix to form the world2view matrix. */
	void updateWorld2ViewMatrix();

	/*! Lets the scene handle keyboard/mouse input.
		\param keyboardHandler Contains information on current state of keyboard and mouse (and mouse move/scroll deltas since last call).
		\param localMousePos Contains mouse position of last left mouse button press or release.
	*/
	void inputEvent(const KeyboardMouseHandler & keyboardHandler, const QPoint & localMousePos);

	/*! Actually renders to the current OpenGL context. */
	void render();

private:
	void generateBuildingGeometry();
	void generateNetworkGeometry();

	/*! Applies filter/highlighting rules and determines color for given surface. */
	QColor color4Surface(const VICUS::Surface & s) const;

	/*! Mouse pick handler: collects all surfaces along the pick line and stores first intersection point's
		coordinates in m_pickPoint. */
	void pick(const QPoint & localMousePos);

	/*! Determine which objects/planes are selected and color them accordingly.
		nearPoint and farPoint define the current ray and are given in model coordinates.
	*/
	void selectNearestObject(const QVector3D & nearPoint, const QVector3D & farPoint);

	/*! Stores viewport geometry. */
	QRect					m_viewPort;

	/*! Stores address to shader program (managed by SceneView). */
	ShaderProgram			*m_gridShader			= nullptr;
	/*! Shader program (managed by SceneView). */
	ShaderProgram			*m_buildingShader		= nullptr;
	/*! Stores address to shader program (managed by SceneView). */
	ShaderProgram			*m_orbitControllerShader = nullptr;
	/*! Stores address to shader program (managed by SceneView). */
	ShaderProgram			*m_coordinateSystemShader = nullptr;

	/*! The projection matrix, updated whenever the viewport geometry changes (in resizeGL() ). */
	QMatrix4x4				m_projection;
	/*! World transformation matrix generator. */
	Transform3D				m_transform;
	/*! Camera position, orientation and lens data. */
	Camera					m_camera;
	/*! Cached world to view transformation matrix. */
	QMatrix4x4				m_worldToView;

	/*! Position of light source, currently very far above. */
	QVector3D				m_lightPos = QVector3D(100,200,2000);
	/*! Light color. */
	QColor					m_lightColor = Qt::white;

	/*! Background color */
	QVector3D				m_background = QVector3D(0.1f, 0.15f, 0.3f);


	// *** Drawable objects ***

	/*! The grid draw object. */
	GridObject				m_gridObject;
	/*! A geometry drawing object (no transparency) for building (room) surfaces.*/
	OpaqueGeometryObject	m_opaqueGeometryObject;
	/*! A geometry drawing object (no transparency) for network elements.*/
	OpaqueGeometryObject	m_networkGeometryObject;
	/*! Indicator for the center of the orbit controller.
		Only visible when m_orbitControllerActive is true.
	*/
	OrbitControllerObject	m_orbitControllerObject;
	/*! The movable coordinate system. */
	CoordinateSystemObject	m_coordinateSystemObject;

	/*! Stores the distance that the mouse has been moved in the last "Left-mouse button down ... release" interval. */
	float					m_mouseMoveDistance = 0.f;
	/*! If true, the orbit controller is active and the rotation axis is to be shown. */
	bool					m_orbitControllerActive = false;
	/*! Holds the origin of the orbit controller coordinates. */
	QVector3D				m_orbitControllerOrigin;

	/*! Indicates whether pick object detection is outdated and should be updated (this flag prevents needless pick
		calculations).
	*/
	bool					m_pickObjectIsOutdated = true;
	/*! Coordinates of last picked point (already including potential snapping). */
	QVector3D				m_pickPoint;
};

} // namespace Vic3D


#endif // VIC3DSCENE_H