FROM ubuntu:20.04
ARG j=7

WORKDIR /opt/mod
COPY ./build/mod-*.tar.gz ./
RUN tar xzf mod-*.tar.gz --strip-components=1

# apt-utils is apparently needed for doing the dpkg path exclude/include
RUN apt-get update -qq                                      \
 && DEBIAN_FRONTEND=noninteractive                          \
    apt install --no-install-recommends -y apt-utils        \
 && echo                                                    \
   'path-exclude /usr/share/doc/*'                          \
   '\npath-include /usr/share/doc/*/copyright'              \
   '\npath-exclude /usr/share/man/*'                        \
   '\npath-exclude /usr/share/groff/*'                      \
   '\npath-exclude /usr/share/info/*'                       \
   '\npath-exclude /usr/share/lintian/*'                    \
   '\npath-exclude /usr/share/linda/*'                      \
   > /etc/dpkg/dpkg.cfg.d/01_nodoc                          \
 && DEBIAN_FRONTEND=noninteractive                          \
    apt install --no-install-recommends -y python3-pip wget \
 && pip3 install -r requirements_nodoc.txt                  \
 && DEBIAN_FRONTEND=noninteractive                          \
    apt install --no-install-recommends -y                  \
    $(bindep -b testing | tr '\n' ' ')                      \
    librsvg2-dev libpango1.0-dev                            \
 && DEBIAN_FRONTEND=noninteractive                          \
    apt install --no-install-recommends -y                  \
    vim less                                                \
 && apt-get clean                                           \
 && rm -rf /var/lib/apt/lists/*

# Graphviz
WORKDIR /opt/graphviz
RUN \
 wget http://graphviz.gitlab.io/pub/graphviz/stable/SOURCES/graphviz.tar.gz \
 && tar -xf graphviz.tar.gz --one-top-level=graphviz --strip-components=1   \
 && cd graphviz                                                             \
 && ./configure                                                             \
 && make -j $j                                                              \
 && make install                                                            \
 && cd ..                                                                   \
 && rm -rf /opt/graphviz

# Boost
# the folder can apparently not be called just 'boost', therefore 'boostDir'
WORKDIR /opt/boostDir
RUN wget                                                                   \
 https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz \
 -O boost.tar.gz
RUN \
 tar -xf boost.tar.gz --one-top-level=boostSrc --strip-components=1        \
 && cd boostSrc                                                            \
 && ./bootstrap.sh --with-python=python3                                   \
 && ./b2 --with-python --with-graph --with-iostreams --with-test -j $j     \
 && ./b2 install                                                           \
 && cd ..                                                                  \
 && rm -rf /opt/boostDir



WORKDIR /opt/mod/build
ENV CXXFLAGS=-Werror
RUN cmake ../ -DBUILD_DOC=no                  \
 -DCMAKE_BUILD_TYPE=Release                   \
 -DCMAKE_MODULE_LINKER_FLAGS="-flto=$j"       \
 -DCMAKE_SHARED_LINKER_FLAGS="-flto=$j"       \
 -DBUILD_TESTING=on                           \
 && make -j $j                                \
 && make tests -j $j                          \
 && make install                              \
 && cp -a ../examples/py /examples            \
 && ctest -j $j --output-on-failure -E cmake_ \
 && rm -rf /opt/mod

WORKDIR /workdir
RUN chmod og+rwX .
