FROM ubuntu:22.04
ARG j=7
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
    echo ". /opt/conda/etc/profile.d/conda.sh" >> ~/.bashrc

# and now ours

# TexLive
RUN apt-get update                                                 \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y              \
    texlive-science texlive-pictures texlive-latex-extra lmodern

WORKDIR /opt/mod
COPY ./build/mod-*.tar.gz ./
RUN tar xzf mod-*.tar.gz --strip-components=1
RUN conda env create -f conda/environment.yaml

RUN conda init bash
SHELL ["/bin/bash", "--login", "-c"]
RUN echo "conda activate mod-env" >> ~/.bashrc

WORKDIR /opt/mod/build
ENV CXXFLAGS=-Werror
RUN conda activate mod-env && conda env list && \
 cmake ../ -DBUILD_DOC=no                     \
 -DCMAKE_BUILD_TYPE=Release                   \
 -DCMAKE_MODULE_LINKER_FLAGS="-flto=$j"       \
 -DCMAKE_SHARED_LINKER_FLAGS="-flto=$j"       \
 -DBUILD_TESTING=on                           \
 -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX         \
 && make -j $j                                \
 && make tests -j $j                          \
 && make install                              \
 && ctest -j $j --output-on-failure -E cmake_
