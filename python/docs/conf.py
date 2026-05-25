# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

# (for local build:
#   cd python/
#   pip install -e .
#   cd docs
#   make html)

project = 'Randompack'
copyright = '2026, Kristján Jónasson'
author = 'Kristján Jónasson'

import pathlib, tomllib

pyproject = pathlib.Path(__file__).parent.parent / "pyproject.toml"
data = tomllib.loads(pyproject.read_text(encoding="utf-8"))
release = data["project"]["version"]

# -- General configuration ---------------------------------------------------

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.napoleon",
    "sphinx.ext.autosummary",
    "sphinx.ext.viewcode",
    "numpydoc",
]

autosummary_generate = True
numpydoc_show_class_members = False
autosummary_imported_members = True
add_module_names = False
autodoc_typehints = "none"
templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'

html_theme_options = {
    "sidebar_hide_name": True,
}

html_static_path = ["_static"]
html_css_files = ["custom.css"]

pygments_style = "default"
pygments_dark_style = "monokai"
