/*
  ==============================================================================

   This file is part of the osci-render GUI module
   Copyright (c) 2026 James H Ball

  ==============================================================================
*/

#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <vector>

#include "osci_gui_config.h"

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.

 BEGIN_JUCE_MODULE_DECLARATION

  ID:                osci_gui
  vendor:            jameshball
  version:           1.0.0
  name:              osci GUI
  description:       Shared JUCE GUI components and styling for osci apps
  website:           https://osci-render.com
  license:           GPLv3
  minimumCppStandard: 20

 dependencies:      juce_core, juce_graphics, juce_gui_basics, juce_gui_extra, juce_audio_processors, juce_audio_utils, juce_animation, melatonin_blur, osci_render_core

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#include <juce_animation/juce_animation.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <melatonin_blur/melatonin_blur.h>
#include <osci_render_core/osci_render_core.h>

#include "lookandfeel/osci_LookAndFeel.h"
#include "lookandfeel/osci_OverlayLookAndFeel.h"
#include "components/osci_ParameterContextMenu.h"
#include "components/osci_BooleanParamCCHelper.h"
#include "components/osci_ToggleAnimationController.h"
#include "components/osci_HoverAnimationMixin.h"
#include "components/osci_AnimatedTextButton.h"
#include "components/osci_OverlayListBox.h"
#include "components/osci_TextEditor.h"
#include "components/osci_FormFieldLabel.h"
#include "components/osci_FormValidation.h"
#include "components/osci_SvgButton.h"
#include "components/osci_CloseButton.h"
#include "components/osci_NotificationComponent.h"
#include "components/osci_UndoRedoComponent.h"
#include "components/osci_SwitchButton.h"
#include "components/osci_SvgSwitchButton.h"
#include "components/osci_DebouncedCallback.h"
#include "components/osci_SliderTextBox.h"
#include "components/osci_LabelledTextBox.h"
#include "components/osci_ValuePopupHelper.h"
#include "components/osci_VerticalTabListComponent.h"
#include "components/osci_ParameterSyncHelper.h"
#include "components/osci_DownloadProgressComponent.h"
#include "components/osci_DownloaderComponent.h"
#include "components/osci_StopwatchComponent.h"
#include "components/osci_VolumeComponent.h"
#include "components/osci_CustomTooltipWindow.h"
#include "components/osci_ScrollFadeOverlay.h"
#include "components/osci_ScrollFadeViewport.h"
#include "components/osci_OverlayComponent.h"
#include "components/osci_CardComponent.h"
#include "components/osci_FileDropZoneComponent.h"
#include "components/osci_ImagePreviewHelpers.h"
#include "components/osci_ImagePreviewComponent.h"
#include "components/osci_ImagePreviewOverlay.h"
#include "components/osci_ErrorOverlay.h"
#include "components/osci_NumberedStepsComponent.h"
#include "components/osci_LicenseHelpOverlay.h"
#include "components/osci_GridItemComponent.h"
#include "components/osci_GridComponent.h"
#include "components/osci_DisabledOverlay.h"
#include "components/list/osci_VListBox.h"
#include "components/list/osci_ComponentList.h"
#include "components/list/osci_DraggableListBox.h"
#include "components/midi/osci_CustomMidiKeyboardComponent.h"
#include "components/parameters/osci_DarkBarPainter.h"
#include "components/parameters/osci_LabelledBarComponent.h"
#include "components/parameters/osci_InlineEditorHelper.h"
#include "components/parameters/osci_InlineValueEditor.h"
#include "components/parameters/osci_DoubleTextBox.h"
#include "components/parameters/osci_ParameterBarComponent.h"
#include "components/parameters/osci_ToggleLabelComponent.h"
#include "components/sliders/osci_RotaryKnobComponent.h"
#include "components/sliders/osci_PhaseSliderComponent.h"
