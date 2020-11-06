/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QMatrix4x4>
#include <QOpenGLTimeMonitor>
#include <QElapsedTimer>

#include "Vic3DOpenGLWindow.h"
#include "Vic3DShaderProgram.h"
#include "Vic3DKeyboardMouseHandler.h"
#include "Vic3DGridObject.h"
#include "Vic3DBoxObject.h"
#include "Vic3DPickLineObject.h"
#include "Vic3DCamera.h"
#include "Vic3DScene.h"

/*! The class SceneView extends the primitive OpenGLWindow
	by adding keyboard/mouse event handling, and rendering of different
	objects (that encapsulate shader programs and buffer object).
*/
class SceneView : public OpenGLWindow {
public:
	SceneView();
	virtual ~SceneView() override;

protected:
	void initializeGL() override;
	void resizeGL(int width, int height) override;
	void paintGL() override;

	// Functions to handle key press and mouse press events, all the work is done in class KeyboardMouseHandler
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

	void pick(const QPoint & globalMousePos);

private:
	/*! Tests, if any relevant input was received and registers a state change. */
	void checkInput();

	/*! This function is called first thing in the paintGL() routine and
		processes input received so far and updates camera position.
	*/
	void processInput();

	/*! Compines camera matrix and project matrix to form the world2view matrix. */
	void updateWorld2ViewMatrix();

	/*! Determine which objects/planes are selected and color them accordingly.
		nearPoint and farPoint define the current ray and are given in model coordinates.
	*/
	void selectNearestObject(const QVector3D & nearPoint, const QVector3D & farPoint);

	/*! If set to true, an input event was received, which will be evaluated at next repaint. */
	bool						m_inputEventReceived;

	/*! The input handler, that encapsulates the event handling code. */
	KeyboardMouseHandler		m_keyboardMouseHandler;

	/*! The projection matrix, updated whenever the viewport geometry changes (in resizeGL() ). */
	QMatrix4x4					m_projection;
	Transform3D					m_transform;	// world transformation matrix generator
	Camera						m_camera;		// Camera position, orientation and lens data
	QMatrix4x4					m_worldToView;	// cached world to view transformation matrix

	/*! All shader programs used in the scene. */
	QList<ShaderProgram>		m_shaderPrograms;

	Vic3DScene					m_mainScene;

	BoxObject					m_boxObject;
	PickLineObject				m_pickLineObject;

	QOpenGLTimeMonitor			m_gpuTimers;
	QElapsedTimer				m_cpuTimer;
};

#endif // SCENEVIEW_H
