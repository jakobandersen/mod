Known Issues
============

.. _issue-fmt:

Error ``<prefix>/share/mod/commonPreamble.fmt made by different executable version`` during post-processing
-----------------------------------------------------------------------------------------------------------

This error message may be printed by ``pdflatex`` when ``post_mod`` is compiling figures.

MØD installs a precompiled Latex format file for speeding up figure compilation.
However, if the Latex installation is upgraded after MØD is installed this error may happen.
To recompile and install the format file again, call the post-processor as
:option:`mod_post --install-format` or
:option:`mod_post --install-format-sudo`.
