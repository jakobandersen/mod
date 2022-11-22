FROM ubuntu:20.04 AS build
# Based on continuumio/miniconda3

#ENV LANG=C.UTF-8 LC_ALL=C.UTF-8
ENV PATH /opt/conda/bin:$PATH

RUN apt-get update --fix-missing                               \
 && apt-get install -y wget bzip2 ca-certificates curl git

RUN wget --quiet https://repo.anaconda.com/miniconda/Miniconda3-py38_4.12.0-Linux-x86_64.sh -O ~/miniconda.sh && \
    /bin/bash ~/miniconda.sh -b -p /opt/conda && \
    rm ~/miniconda.sh && \
    /opt/conda/bin/conda clean -tipsy && \
    ln -s /opt/conda/etc/profile.d/conda.sh /etc/profile.d/conda.sh && \
    echo ". /opt/conda/etc/profile.d/conda.sh" >> ~/.bashrc && \
    echo "conda activate base" >> ~/.bashrc

# and now ours

# TexLive
RUN apt-get update                                                 \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y              \
    texlive-science texlive-pictures texlive-latex-extra lmodern

RUN conda install conda-build conda-verify

ARG version
ENV MOD_CONDA_BUILD_VERSION=$version

WORKDIR /opt/mod
COPY ./build/mod-*.tar.gz ./build/
COPY ./conda/build.sh ./conda/
COPY ./conda/meta.yaml ./conda/
COPY ./conda/conda_build_config.yaml ./conda/
RUN conda build -c jakobandersen -c conda-forge ./conda


###############################################################################
# Test
###############################################################################

FROM ubuntu:20.04 AS test
# Based on continuumio/miniconda3

#ENV LANG=C.UTF-8 LC_ALL=C.UTF-8
ENV PATH /opt/conda/bin:$PATH

RUN apt-get update --fix-missing                               \
 && apt-get install -y wget bzip2 ca-certificates curl git

RUN wget --quiet https://repo.anaconda.com/miniconda/Miniconda3-4.5.11-Linux-x86_64.sh -O ~/miniconda.sh && \
    /bin/bash ~/miniconda.sh -b -p /opt/conda && \
    rm ~/miniconda.sh && \
    /opt/conda/bin/conda clean -tipsy && \
    ln -s /opt/conda/etc/profile.d/conda.sh /etc/profile.d/conda.sh && \
    echo ". /opt/conda/etc/profile.d/conda.sh" >> ~/.bashrc && \
    echo "conda activate base" >> ~/.bashrc

# and now ours

# TexLive
RUN apt-get update                                                 \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y              \
    texlive-science texlive-pictures texlive-latex-extra lmodern

RUN conda install conda-build conda-verify


COPY --from=build /opt/conda/conda-bld/linux-64/mod-*.tar.bz2      \
                  /opt/conda-bld/linux-64/

RUN conda index /opt/conda-bld
RUN conda install -c file:///opt/conda-bld -c jakobandersen -c conda-forge mod

WORKDIR /opt/test
COPY conda/test.py ./

RUN mod -f test.py
RUN mod_post --install-format
RUN mod -f test.py
