# Configuration file for the Sphinx documentation builder.

project = "BadBishop Chess Engine"
copyright = "2026, BadBishop Contributors"
author = "BadBishop Contributors"
release = "0.9.0"

extensions = [
    "myst_parser",
    "sphinx.ext.mathjax",
    "sphinx_copybutton",
]

# Enable Markdown features: $math$, $$math$$, tables, task lists, and colon fences
myst_enable_extensions = [
    "dollarmath",
    "amsmath",
    "colon_fence",
    "deflist",
    "tasklist",
]
myst_heading_anchors = 3

source_suffix = {
    ".md": "markdown",
}

master_doc = "index"
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# HTML Output Theme (Furo provides built-in dark/light mode and sidebar navigation)
html_theme = "furo"
html_title = "BadBishop C99 Engine Docs"