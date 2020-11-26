# Project file for SIM-VICUS
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = SIM-VICUS
TEMPLATE = app

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT += xml opengl network printsupport widgets

CONFIG += c++11

unix {
        QMAKE_CXXFLAGS += -std=c++11
}

LIBS += -L../../../lib$${DIR_PREFIX} \
                -lIBK \
                -lIBKMK \
                -lNandrad \
                -lQtExt \
                -lQuaZIP \
                -lTiCPP \
                -lVicus

win32 {
        LIBS += -luser32
        LIBS += -lopengl32
}
linux {
        LIBS += -lGLU -lGL
}
mac {
        LIBS += -framework OpenGL
}

INCLUDEPATH = \
        ../../src \
        ../../src/actions \
        ../../src/core3D \
        ../../../externals/IBK/src \
        ../../../externals/IBKMK/src \
        ../../../externals/Nandrad/src \
        ../../../externals/Vicus/src \
        ../../../externals/TiCPP/src \
        ../../../externals/QuaZIP/src \
        ../../../externals/QuaZIP/src/zlib \
        ../../../externals/QtExt/src

DEPENDPATH = $${INCLUDEPATH}

win32 {
        PRE_TARGETDEPS +=   $$PWD/../../../externals/lib$${DIR_PREFIX}/IBK.lib \
                                                $$PWD/../../../externals/lib$${DIR_PREFIX}/CCM.lib \
                                                $$PWD/../../../externals/lib$${DIR_PREFIX}/QtExt.lib \
                                                $$PWD/../../../externals/lib$${DIR_PREFIX}/qwt6.lib \
                                                $$PWD/../../../externals/lib$${DIR_PREFIX}/QuaZIP.lib \
                                                $$PWD/../../../externals/lib$${DIR_PREFIX}/Vicus.lib \
                                                $$PWD/../../../externals/lib$${DIR_PREFIX}/Nandrad.lib \
                                                $$PWD/../../../externals/lib$${DIR_PREFIX}/TiCPP.lib \
                                                $$PWD/../../../externals/lib$${DIR_PREFIX}/IBKMK.lib
}


SOURCES += \
        ../../src/SVPreferencesPageStyle.cpp \
        ../../src/actions/SVUndoAddNetwork.cpp \
        ../../src/actions/SVUndoCommandBase.cpp \
        ../../src/actions/SVUndoProject.cpp \
        ../../src/core3D/Vic3DCoordinateSystemObject.cpp \
        ../../src/core3D/Vic3DGeometryHelpers.cpp \
        ../../src/core3D/Vic3DGridObject.cpp \
        ../../src/core3D/Vic3DKeyboardMouseHandler.cpp \
        ../../src/core3D/Vic3DNewPolygonObject.cpp \
        ../../src/core3D/Vic3DOpaqueGeometryObject.cpp \
        ../../src/core3D/Vic3DOpenGLWindow.cpp \
        ../../src/core3D/Vic3DOrbitControllerObject.cpp \
        ../../src/core3D/Vic3DPickLineObject.cpp \
        ../../src/core3D/Vic3DPickObject.cpp \
        ../../src/core3D/Vic3DScene.cpp \
        ../../src/core3D/Vic3DSceneView.cpp \
        ../../src/core3D/Vic3DShaderProgram.cpp \
        ../../src/core3D/Vic3DTransform3D.cpp \
        ../../src/main.cpp \
        ../../src/SVAboutDialog.cpp \
        ../../src/SVButtonBar.cpp \
        ../../src/SVConstants.cpp \
        ../../src/SVDebugApplication.cpp \
        ../../src/SVGeometryView.cpp \
        ../../src/SVLogFileDialog.cpp \
        ../../src/SVLogWidget.cpp \
        ../../src/SVMainWindow.cpp \
        ../../src/SVMessageHandler.cpp \
        ../../src/SVNavigationTreeWidget.cpp \
        ../../src/SVNetworkImportDialog.cpp \
        ../../src/SVPostProcBindings.cpp \
        ../../src/SVPostProcHandler.cpp \
        ../../src/SVPreferencesDialog.cpp \
        ../../src/SVPreferencesPageTools.cpp \
        ../../src/SVProjectHandler.cpp \
        ../../src/SVSettings.cpp \
        ../../src/SVStyle.cpp \
        ../../src/SVThreadBase.cpp \
        ../../src/SVWelcomeScreen.cpp

HEADERS  += \
        ../../src/SVPreferencesPageStyle.h \
        ../../src/actions/SVUndoAddNetwork.h \
        ../../src/actions/SVUndoCommandBase.h \
        ../../src/actions/SVUndoProject.h \
        ../../src/core3D/Vic3DCamera.h \
        ../../src/core3D/Vic3DConstants.h \
        ../../src/core3D/Vic3DCoordinateSystemObject.h \
        ../../src/core3D/Vic3DGeometryHelpers.h \
        ../../src/core3D/Vic3DGridObject.h \
        ../../src/core3D/Vic3DKeyboardMouseHandler.h \
        ../../src/core3D/Vic3DNewPolygonObject.h \
        ../../src/core3D/Vic3DOpaqueGeometryObject.h \
        ../../src/core3D/Vic3DOpenGLWindow.h \
        ../../src/core3D/Vic3DOrbitControllerObject.h \
        ../../src/core3D/Vic3DPickLineObject.h \
        ../../src/core3D/Vic3DPickObject.h \
        ../../src/core3D/Vic3DScene.h \
        ../../src/core3D/Vic3DSceneView.h \
        ../../src/core3D/Vic3DShaderProgram.h \
        ../../src/core3D/Vic3DTransform3D.h \
        ../../src/core3D/Vic3DVertex.h \
        ../../src/SVAboutDialog.h \
        ../../src/SVButtonBar.h \
        ../../src/SVConstants.h \
        ../../src/SVDebugApplication.h \
        ../../src/SVGeometryView.h \
        ../../src/SVLogFileDialog.h \
        ../../src/SVLogWidget.h \
        ../../src/SVMainWindow.h \
        ../../src/SVMessageHandler.h \
        ../../src/SVNavigationTreeWidget.h \
        ../../src/SVNetworkImportDialog.h \
        ../../src/SVPostProcBindings.h \
        ../../src/SVPostProcHandler.h \
        ../../src/SVPreferencesDialog.h \
        ../../src/SVPreferencesPageTools.h \
        ../../src/SVProjectHandler.h \
        ../../src/SVSettings.h \
        ../../src/SVStyle.h \
        ../../src/SVThreadBase.h \
        ../../src/SVWelcomeScreen.h

FORMS    += \
        ../../src/SVAboutDialog.ui \
        ../../src/SVLogFileDialog.ui \
        ../../src/SVMainWindow.ui \
        ../../src/SVNavigationTreeWidget.ui \
        ../../src/SVNetworkImportDialog.ui \
        ../../src/SVPreferencesDialog.ui \
        ../../src/SVPreferencesPageStyle.ui \
        ../../src/SVPreferencesPageTools.ui \
        ../../src/SVWelcomeScreen.ui

TRANSLATIONS += ../../resources/translations/SIM-VICUS_de.ts
CODECFORSRC = UTF-8

RESOURCES += ../../resources/SIM-VICUS.qrc \
        ../../resources/qdarkstyle/style.qrc \
        ../../src/shaders/shaders.qrc





