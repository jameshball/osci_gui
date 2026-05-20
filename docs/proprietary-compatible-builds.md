# Proprietary-Compatible Builds

`osci_gui` is publicly distributed under the GPLv3 license declared by the JUCE module and repository license files.

This module can also contain code paths intended for private commercial use by James H Ball or by proprietary products he controls. Those paths are governed by the root policy in `../../../docs/proprietary-compatible-builds.md`.

This is an engineering policy document, not legal advice.

## Build Mode

Set `OSCI_PROPRIETARY_BUILD=1` when compiling a proprietary consumer or verifying that this module avoids incompatible dependencies.

This is a build-mode boundary only. It does not replace ownership records, private licensing terms, third-party notices, or commercial licenses for framework dependencies.

In this mode, `osci_gui` code should:

- compile without GPL-only or unresolved-provenance dependencies
- avoid assets whose commercial-use provenance is unclear
- keep reusable APIs independent of osci-render application-only UI and asset assumptions
- continue to allow permissively licensed dependencies where their license terms permit proprietary use

Project-owned assets may remain available in proprietary-compatible builds; only assets with unclear or incompatible rights should be excluded, replaced, or separately licensed.

When `OSCI_PROPRIETARY_BUILD` is unset or `0`, GPL osci-render builds may continue to use GPL-compatible dependencies and GPL-distributed application assets.

## Optional Features

The base `osci_gui` module should avoid compiling optional feature code by default.

- Set `OSCI_GUI_ENABLE_VISUALISER=1` to compile the visualiser renderer.
- Projects enabling the visualiser must include its optional module dependencies, currently `osci_render_core` and JUCE OpenGL support.
- Set `OSCI_GUI_ENABLE_CHOWDSP_RESAMPLING=1` to enable ChowDSP-backed upsampling for visualiser code.

Flag defaults and proprietary-mode conflicts are centralized in `../osci_gui_config.h`. Visualiser-specific dependency checks live in the visualiser support headers.

The ChowDSP feature flag must remain off in proprietary-compatible builds unless that dependency is separately cleared for proprietary use and the guard is intentionally updated.

Project-owned code and assets may be added to this module over time. Bundled or app-provided assets still need clear commercial-use provenance for proprietary products.

## Contribution Policy

Do not accept third-party contributions into proprietary-compatible paths unless the contribution is under terms compatible with private commercial use. Otherwise, keep the contribution outside `OSCI_PROPRIETARY_BUILD` builds or reimplement it independently.

## References

- Root policy: `../../../docs/proprietary-compatible-builds.md`
