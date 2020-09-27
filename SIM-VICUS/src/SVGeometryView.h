#ifndef SVGEOMETRYVIEW_H
#define SVGEOMETRYVIEW_H

#include <QWidget>

class SceneView;

/*! The main geometry view.
	The 3D scene view is embedded into this widget.
*/
class SVGeometryView : public QWidget {
	Q_OBJECT
public:
	explicit SVGeometryView(QWidget *parent = nullptr);

private:
	// The scene view, that shows our world and allows navigation
	SceneView * m_sceneView;

};

#endif // SVGEOMETRYVIEW_H