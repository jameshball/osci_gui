#pragma once

#include "../osci_gui_config.h"

#if !OSCI_GUI_ENABLE_VISUALISER
#error "Define OSCI_GUI_ENABLE_VISUALISER=1 before including osci_gui visualiser headers."
#endif

#if !__has_include(<osci_render_core/osci_render_core.h>)
#error "OSCI_GUI_ENABLE_VISUALISER=1 requires the osci_render_core module in the consuming project."
#endif
#include <osci_render_core/osci_render_core.h>
