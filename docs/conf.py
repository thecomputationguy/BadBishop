project = "BadBishop Chess Engine"
copyright = "2026, BadBishop Contributors"
author = "BadBishop Contributors"
release = "0.9.0"

extensions = [
    "myst_parser",
    "sphinx.ext.mathjax",
    "sphinx_copybutton",
    "breathe",
    "exhale",
]

myst_enable_extensions = [
    "dollarmath",
    "amsmath",
    "colon_fence",
    "deflist",
    "tasklist",
]
myst_heading_anchors = 3

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}
master_doc = "index"
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# Breathe Configuration
breathe_projects = {"BadBishop": "../build/doxygen/xml"}
breathe_default_project = "BadBishop"

# Exhale Configuration (Auto-generates full C API Reference)
exhale_args = {
    "containmentFolder": "./api",
    "rootFileName": "library_root.rst",
    "doxygenStripFromPath": "..",
    "rootFileTitle": "C99 API Reference",
    "createTreeView": True,
    "exhaleExecutesDoxygen": True,
    "exhaleDoxygenStdin": """
        INPUT = ../src
        OPTIMIZE_OUTPUT_FOR_C = YES
        EXTRACT_ALL = YES
        EXTRACT_STATIC = YES
    """,
}

html_theme = "furo"
html_title = "BadBishop C99 Engine Docs"