ARG miniforgeVersion=25.3.1-0
# Use the oldest Ubuntu possible, to make sure the system Python is the oldest
# we are trying to build against. Otherwise CMake seems to find the system Python
# instead of the Conda Python.
FROM ubuntu:22.04 AS build
ARG miniforgeVersion

#ENV LANG=C.UTF-8 LC_ALL=C.UTF-8
ENV PATH /opt/conda/bin:$PATH

RUN apt-get update --fix-missing                               \
 && apt-get install -y wget bzip2 ca-certificates curl git

RUN wget https://github.com/conda-forge/miniforge/releases/download/${miniforgeVersion}/Miniforge3-${miniforgeVersion}-Linux-$(uname -m).sh -O ~/miniforge.sh \
 && /bin/bash ~/miniforge.sh -b -p /opt/conda \
 && rm ~/miniforge.sh \
 && /opt/conda/bin/conda clean -afy \
 && ln -s /opt/conda/etc/profile.d/conda.sh /etc/profile.d/conda.sh \
 && echo ". /opt/conda/etc/profile.d/conda.sh" >> ~/.bashrc \
 && echo "conda activate base" >> ~/.bashrc

# TexLive
RUN apt-get update                                                 \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y              \
    texlive-science texlive-pictures texlive-latex-extra lmodern

RUN conda install "conda-build>=25.9"

ARG version
ENV MOD_CONDA_BUILD_VERSION=$version

WORKDIR /opt/mod
COPY ./build/mod-*.tar.gz ./build/
COPY ./conda/build.sh ./conda/
COPY ./conda/meta.yaml ./conda/
COPY ./conda/conda_build_config.yaml ./conda/
RUN conda build ./conda

RUN echo "END CONDA BUILD"

###############################################################################
# Test
###############################################################################

RUN echo "START INSTALLATION TEST"

FROM ubuntu:22.04 AS test
ARG miniforgeVersion

#ENV LANG=C.UTF-8 LC_ALL=C.UTF-8
ENV PATH /opt/conda/bin:$PATH

RUN apt-get update --fix-missing                               \
 && apt-get install -y wget bzip2 ca-certificates curl git

RUN wget https://github.com/conda-forge/miniforge/releases/download/${miniforgeVersion}/Miniforge3-${miniforgeVersion}-Linux-$(uname -m).sh -O ~/miniforge.sh \
 && /bin/bash ~/miniforge.sh -b -p /opt/conda \
 && rm ~/miniforge.sh \
 && /opt/conda/bin/conda clean -afy \
 && ln -s /opt/conda/etc/profile.d/conda.sh /etc/profile.d/conda.sh \
 && echo ". /opt/conda/etc/profile.d/conda.sh" >> ~/.bashrc \
 && echo "conda activate base" >> ~/.bashrc

# TexLive
RUN apt-get update                                                 \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y              \
    texlive-science texlive-pictures texlive-latex-extra lmodern

RUN conda install "conda-build>=25.9"


COPY --from=build /opt/conda/conda-bld/linux-64/mod-*.conda        \
                  /opt/conda-bld/linux-64/

RUN conda index /opt/conda-bld
RUN conda install -c file:///opt/conda-bld -c jakobandersen -c conda-forge mod

WORKDIR /opt/test
COPY conda/test.py ./

RUN mod -f test.py
RUN mod_post --install-format
RUN mod -f test.py

RUN echo "END INSTALLATION TEST"
