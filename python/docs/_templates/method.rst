{% set short = fullname.split('.')[-2:] | join('.') %}
{{ short | escape | underline }}

.. currentmodule:: {{ module }}

.. automethod:: {{ fullname }}
