import Qt.labs.platform 1.1
import QtQuick 2.12
import QtQuick.Controls 2.12
import ChaosKit 1.0

ApplicationWindow {
  height: 768
  title: "ChaosKit"
  visible: true
  width: 1024

  Snackbar {
    id: snackbar
  }

  function openSnackbar(text) {
    snackbar.text = text;
    snackbar.open();
  }

  ExportImageDialog {
    id: exportImageDialog
    onAccepted: {
      const fileName = Utilities.urlToLocalPath(file);
      systemPreview.grabToImage(result => {
        if (result.saveToFile(fileName)) {
          openSnackbar("Image exported");
          console.log("Exported image to", fileName);
        } else {
          openSnackbar("Failed to export image");
          console.error("Could not export image to", fileName);
        }
      });
    }
  }

  MainMenu {
    onExportImage: {
      exportImageDialog.open()
    }
  }

  SystemPreview {
    id: systemPreview
    anchors.centerIn: parent
    transform: Translate {
      x: documentEditor.opened ? -documentEditor.width * 0.33 : 0
    }

    gamma: documentModel.documentProxy.gamma
    exposure: documentModel.documentProxy.exposure
    vibrancy: documentModel.documentProxy.vibrancy

    Component.onCompleted: {
      running = true;
    }
  }

  DocumentEditor {
    id: documentEditor
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.bottom: parent.bottom
  }

  Button {
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.margins: Theme.units(4)
    text: "Debug Source"
    outlined: true

    onClicked: {
      astInspectorLoader.active = true;
    }

    Loader {
      id: astInspectorLoader
      active: false
      sourceComponent: AstInspector {
        anchors.centerIn: Overlay.overlay
        source: documentModel.debugSource

        onClosed: {
          astInspectorLoader.active = false;
        }
      }
    }
  }

  Fab {
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.margins: Theme.units(4)
    enabled: true
    iconName: "random"
    ToolTip.text: "Randomize the image"
    ToolTip.visible: hovered

    onClicked: {
      documentModel.randomizeSystem();
    }
  }
}
