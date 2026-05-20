/*
  ==============================================================================

   This file is part of the osci-render GUI module
   Copyright (c) 2026 James H Ball

  ==============================================================================
*/

#pragma once

#include <cmath>
#include <memory>
#include <optional>
#include <vector>

#ifndef OSCI_PROPRIETARY_BUILD
#define OSCI_PROPRIETARY_BUILD 0
#endif

#ifndef OSCI_GUI_ENABLE_VISUALISER
#define OSCI_GUI_ENABLE_VISUALISER 0
#endif

#ifndef OSCI_GUI_ENABLE_CHOWDSP_RESAMPLING
#define OSCI_GUI_ENABLE_CHOWDSP_RESAMPLING 0
#endif

#if OSCI_PROPRIETARY_BUILD && OSCI_GUI_ENABLE_CHOWDSP_RESAMPLING
#error "OSCI_GUI_ENABLE_CHOWDSP_RESAMPLING cannot be enabled in OSCI_PROPRIETARY_BUILD."
#endif

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

  dependencies:      juce_core, juce_graphics, juce_gui_basics, juce_gui_extra, juce_audio_processors, juce_animation, melatonin_blur

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#include <juce_animation/juce_animation.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <melatonin_blur/melatonin_blur.h>

#include "lookandfeel/osci_LookAndFeel.h"
#include "lookandfeel/osci_OverlayLookAndFeel.h"
#include "components/osci_ToggleAnimationController.h"
#include "components/osci_HoverAnimationMixin.h"
#include "components/osci_OverlayListBox.h"
#include "components/osci_TextEditor.h"
#include "components/osci_SvgButton.h"
#include "components/osci_OverlayComponent.h"
#include "components/osci_ErrorOverlay.h"
#include "components/osci_NumberedStepsComponent.h"
#include "components/osci_LicenseHelpOverlay.h"
#include "components/osci_ScrollFadeOverlay.h"
#include "components/osci_ScrollFadeViewport.h"
#include "components/osci_GridItemComponent.h"
#include "components/osci_GridComponent.h"
