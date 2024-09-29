set(SOURCE_FILES
  CharacterSet.cpp
  CharacterSet.hpp
  Constants.cpp
  Constants.hpp
  FontModel.cpp
  FontModel.hpp
  FontGenContext.cpp
  FontGenContext.hpp
  FontGenWorkerThread.cpp
  FontGenWorkerThread.hpp
  FontPage.cpp
  FontPage.hpp
  GeneratedFont.cpp
  GeneratedFont.hpp
  Glyph.hpp
  ImageCache.cpp
  ImageCache.hpp
  Main.cpp
  MaxRectsBinPack.cpp
  MaxRectsBinPack.hpp
  QtImageUtil.cpp
  QtImageUtil.hpp
  QtStringUtil.cpp
  QtStringUtil.hpp
  widgets/AngleDial.cpp
  widgets/AngleDial.hpp
  widgets/CharacterSetSelectionWidget.cpp
  widgets/CharacterSetSelectionWidget.hpp
  widgets/ClickableLabel.cpp
  widgets/ClickableLabel.hpp
  widgets/ColorPickerWidget.cpp
  widgets/ColorPickerWidget.hpp
  widgets/ColorWheelWidget.cpp
  widgets/ColorWheelWidget.hpp
  widgets/ComboBox.cpp
  widgets/ComboBox.hpp
  widgets/FillOptionsWidget.cpp
  widgets/FillOptionsWidget.hpp
  widgets/FontEditorWidget.cpp
  widgets/FontEditorWidget.hpp
  widgets/FontImageTypeComboBox.cpp
  widgets/FontImageTypeComboBox.hpp
  widgets/FontSelectionWidget.cpp
  widgets/FontSelectionWidget.hpp
  widgets/FontWidget.cpp
  widgets/FontWidget.hpp
  widgets/GradientEditorWidget.cpp
  widgets/GradientEditorWidget.hpp
  widgets/ListWidget.cpp
  widgets/ListWidget.hpp
  widgets/RightAlignedLabel.cpp
  widgets/RightAlignedLabel.hpp
  widgets/ShortFilenameLineEdit.cpp
  widgets/ShortFilenameLineEdit.hpp
  widgets/Slider.cpp
  widgets/Slider.hpp
  widgets/SpinBox.cpp
  widgets/SpinBox.hpp
  widgets/StatusLabel.cpp
  widgets/StatusLabel.hpp
  widgets/TextPreviewWidget.cpp
  widgets/TextPreviewWidget.hpp
  widgets/WaitingSpinner.cpp
  widgets/WaitingSpinner.hpp
  windows/AboutDialog.cpp
  windows/AboutDialog.hpp
  windows/CharacterSetsDialog.cpp
  windows/CharacterSetsDialog.hpp
  windows/FontVariationDialog.cpp
  windows/FontVariationDialog.hpp
  windows/MainWindow.cpp
  windows/MainWindow.hpp
)

set(UI_FILES
  widgets/FillOptionsWidget.ui
  widgets/FontEditorWidget.ui
  widgets/FontWidget.ui
  widgets/TextPreviewWidget.ui
  windows/AboutDialog.ui
  windows/CharacterSetsDialog.ui
  windows/FontVariationDialog.ui
  windows/MainWindow.ui
)

set(QRC_FILES
  resources/GeneralResources.qrc
  resources/UIResources.qrc
)

set(MISC_FILES files.cmake)

source_group("Source Files" FILES ${SOURCE_FILES})
source_group(UI FILES ${UI_FILES})
source_group(QRC FILES ${QRC_FILES})
source_group(Misc FILES ${MISC_FILES})

file(GLOB_RECURSE RESOURCE_FILES "resources/*.*")
source_group("Resources" FILES ${RESOURCE_FILES})

list(APPEND SOURCE_FILES ${RESOURCE_FILES} ${MISC_FILES})


