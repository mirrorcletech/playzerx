# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import os
import sys
sys.path.insert(0, os.path.abspath('.'))


project = 'PlayzerX'
copyright = '2025, Mirrorcle Technologies, Inc'
author = 'Mirrorcle Technologies, Inc'
release = '2.0.1.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.doctest',
    'sphinx.ext.intersphinx',
    'sphinx.ext.todo',
    'sphinx.ext.coverage',
    'sphinx.ext.ifconfig',
    'sphinx.ext.autosectionlabel',
    'sphinx_copybutton',
    'sphinx_inline_tabs',
    'sphinxext.opengraph',
    'breathe'
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

breathe_projects = { "playzerx-docs": "./xml" }
breathe_default_project = "playzerx-docs"



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_book_theme'
html_theme_options = {
    "path_to_docs": "docs/",
    "repository_url": "https://github.com/mirrorcletech/playzerx",
    "repository_branch": "master",
    "use_repository_button": True,
    "logo": {
        "image_light": "_static/playzerx_logo.png",
        "image_dark": "_static/playzerx_logo_dark.png"
    }
}

html_static_path = ['_static']
# html_logo = '_static/playzerx_logo.png'
html_css_files = ['custom.css']

