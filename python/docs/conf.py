# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Randompack'
copyright = '2026, Kristján Jónasson'
author = 'Kristján Jónasson'

import pathlib, tomllib

pyproject = pathlib.Path(__file__).parent.parent / "pyproject.toml"
data = tomllib.loads(pyproject.read_text(encoding="utf-8"))
release = data["project"]["version"]

# -- General configuration ---------------------------------------------------

import os
import sys
sys.path.insert(0, os.path.abspath(".."))

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.napoleon",
    "sphinx.ext.autosummary",
    "sphinx.ext.viewcode",
    "numpydoc",
]

autosummary_generate = True
numpydoc_show_class_members = False
autosummary_generate = True
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


print("SPHINX srcdir:", os.getcwd())
print("templates_path:", templates_path)
print("abs templates:", [os.path.abspath(p) for p in templates_path])
print("method template exists:",
      os.path.exists(os.path.join(os.path.abspath(templates_path[0]), "autosummary", "method.rst")))
