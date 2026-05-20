#pragma once

#include "osci_VisualiserConfig.h"

#if OSCI_GUI_ENABLE_CHOWDSP_RESAMPLING
#if !__has_include(<chowdsp_dsp_utils/chowdsp_dsp_utils.h>)
#error "OSCI_GUI_ENABLE_CHOWDSP_RESAMPLING=1 requires the ChowDSP modules in the consuming project."
#endif
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#endif

#if !__has_include(<juce_opengl/juce_opengl.h>)
#error "OSCI_GUI_ENABLE_VISUALISER=1 requires the JUCE OpenGL module in the consuming project."
#endif
#include <juce_opengl/juce_opengl.h>
