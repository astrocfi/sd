# Sphinx configuration for the Sd developer guide.
#
# The guide is for people who work on Sd: its build, its tests, and its CI.
# The manuals for callers are the Texinfo files in doc/, which this does not
# build. scripts/build-dev-guide.sh builds the guide into .build/dev-guide, with
# every warning an error, as CI does.

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# -- Project information -----------------------------------------------------

project = 'Sd Developer Guide'
author = 'the Sd developers'
copyright = 'the Sd developers'


def _sd_version():
    """Return the program version from src/sdmain.cpp, or '' if it is missing."""
    try:
        text = (ROOT / 'src' / 'sdmain.cpp').read_text(encoding='latin-1')
    except OSError:
        return ''
    match = re.search(r'#define\s+VERSION_STRING\s+"([^"]+)"', text)
    return match.group(1) if match else ''


release = _sd_version()
version = release

# -- General configuration ---------------------------------------------------

# The pages are Markdown (MyST), so the Markdown checks in scripts/ read them
# as they read the rest of the developer documentation.
extensions = [
    'myst_parser',
]
source_suffix = {
    '.md': 'markdown',
}
myst_enable_extensions = [
    'colon_fence',
    'deflist',
]
# Give every heading down to the third level an anchor that a link can name,
# as in [the deploy key](regression.md#the-deploy-key).
myst_heading_anchors = 3

exclude_patterns = [
    'requirements.txt',
]

# Report a reference that does not resolve.
nitpicky = True

# -- Options for HTML output -------------------------------------------------

html_theme = 'sphinx_rtd_theme'
html_theme_options = {
    'navigation_depth': 4,
    'collapse_navigation': False,
}
html_title = 'Sd Developer Guide'
html_show_sourcelink = False
