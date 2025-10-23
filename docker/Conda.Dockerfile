FROM ubuntu:22.04
ARG miniforgeVersion=25.3.1-0
ARG j=7

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

WORKDIR /opt/mod
COPY ./build/mod-*.tar.gz ./
RUN tar xzf mod-*.tar.gz --strip-components=1
RUN conda env create -f conda/environment.yml

RUN conda init bash
SHELL ["/bin/bash", "--login", "-c"]
RUN echo "conda activate mod-env" >> ~/.bashrc

WORKDIR /opt/mod/build
ENV CXXFLAGS=-Werror
RUN conda activate mod-env && conda env list && \
 cmake ../ -DBUILD_DOC=no                     \
 -DWITH_GUROBI=no -DWITH_CPLEX=no             \
 -DCMAKE_BUILD_TYPE=Release                   \
 -DCMAKE_MODULE_LINKER_FLAGS="-flto=$j"       \
 -DCMAKE_SHARED_LINKER_FLAGS="-flto=$j"       \
 -DBUILD_TESTING=on                           \
 -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX         \
 && make -j $j                                \
 && make tests -j $j                          \
 && make install                              \
 && ctest -j $j --output-on-failure -E cmake_
