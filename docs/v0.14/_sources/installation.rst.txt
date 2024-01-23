.. _installation:

.. py:currentmodule:: mod
.. cpp:namespace:: mod

Installation
============

Currently there are the following options for installing the
package:

- :ref:`compiling`
- Installing via Conda.
- Running via a pre-compiled Docker image.


Conda
-----

This is currently only available for Linux.

1. Install Latex on your system.
   E.g., see the provided ``bindep.txt`` file for which
   TeX Live packages are needed for a selection of popular
   Linux distribuions.
2. ``conda install -c jakobandersen -c conda-forge mod``
3. (Optional, but recommended) Install a pre-compiled Latex
   format file.
   During figure generation by :ref:`mod_post` a Latex format
   file is needed. It depends on your Latex installation and
   can therefore not be provided in the Conda package.
   It will then be compiled at each invocation. To check,
   try running, e.g., ``mod -e "smiles('O').print()"``. 
   If the post-processor output has a line with
   ``compileFmt``, the format file is not installed.

   Run ``mod_post --install-format`` to install the format
   file (or ``mod_post --install-format-sudo`` if you don't
   own the folder with the Conda installation).


Docker
------

A Docker image with an installation is available at
`Docker Hub <https://hub.docker.com/r/jakobandersen/mod>`__,
so getting started can be done simply via

.. code-block:: bash

	docker run -it jakobandersen/mod

It will put you into an empty folder ``/workDir``.
The image is build with a copy of the :ref:`examples` in ``/examples``.

In order to exchange data with the container it may be convient to start the
container with a local folder mounted, for example the current folder

.. code-block:: bash

	docker run -it -v $(pwd):/workdir jakobandersen/mod

However, the user running inside the container is ``root`` so all files you
create inside the shared folder will have ``root`` as owner and group.
Therefore, it may be better to tell Docker to run with your own user and group
inside

.. code-block:: bash

	docker run -it -v $(pwd):/workdir -u $(id -u):$(id -g) jakobandersen/mod

Note though, that your user/group probably doesn't exist in the container so
you may get some warnings about the group not existing and that the user does
not have a name. This is however usually not a problem.

As a final optimization, every time you execute the command above, you create
a new container. You can use ``docker container prune`` to delete stopped
containers. Assuming you only work within the shared ``/workdir`` there is
no particular reason for the container to hang around once you exit,
so you can add ``--rm`` to the command to let Docker automatically delete it
once you exit.
We thus arrive at the following full command

.. code-block:: bash

	docker run -it --rm -v $(pwd):/workdir -u $(id -u):$(id -g) jakobandersen/mod

As a quick test, you can try running ``mod -e "smiles('O').print()"`` inside
the container. When it's done, then outside the container you should now be
able to open the printed summary ``summary/summary.pdf``.
You can thus use the ``mod`` command inside the container, but otherwise
do all file manipulation outside the container.
