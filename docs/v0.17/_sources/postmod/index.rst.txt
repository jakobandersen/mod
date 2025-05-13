.. _mod_post:

****************************
PostMØD
****************************

.. program:: mod_post

The post processor, ``mod_post``, is used for compiling figures and a summary
document of the data in the ``out/`` folder. As default it is automatically
invoked by the wrapper script.

The script does the following:

#. Clear the folder ``summary/``. If it does not exist, it is created.
#. Source the command file ``out/post.sh`` and create ``summary/Makefile``
   (and a bunch of additional Makefile files it includes).
#. Run ``make -f summary/Makefile all`` (with a few more arguments).
   The Makefile will compile figures and generate a Latex file.
   A master Latex docuemnt is compiled to ``summary/summary.pdf``.
   The master document includes the generated Latex file.


Usage
#####

.. code-block:: bash

  mod_post [options]


Options
-------

.. option:: --help, -h

  Shows the help output of the script.

.. option:: -j <N>

  When running ``make``, use ``-j <N>`` as additional arguments.
  This parameter defaults to 2, unless :envvar:`MOD_NUM_POST_THREADS`
  is set.

.. option:: --install-format
            --install-format-sudo

  Compile the common Latex preamble used for figure generation
  and install it. Use the ``-sudo`` version to install using ``sudo``.
  This may be useful to do if you encounter the issue about the format being
  :ref:`made by different executable version <issue-fmt>`,
  or if MØD was configured to not install the format file
  (``-DBUILD_POST_MOD_FMT=off``).


Environtment Variables
----------------------

.. envvar:: MOD_NUM_POST_THREADS

  If defined, the value of this variable is used for ``-j`` when invoking
  ``make``.
  When ``-j <N>`` is given to the script, this variable is ignored.


Useful API References
#####################

The API of libMØD and PyMØD includes functionality for injecting code and
modifying the behaviour of PostMØD and generated summaries.
See :doc:`libMØD (Post.hpp) </libmod/Post>`
and :doc:`PyMØD (Post) </pymod/Post>` for the documentation of these functions.

