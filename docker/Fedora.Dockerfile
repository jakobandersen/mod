FROM fedora:36
ARG j=7

WORKDIR /opt/mod
COPY ./build/mod-*.tar.gz ./
RUN tar xzf mod-*.tar.gz --strip-components=1

RUN dnf install -y                            \
    python3-pip                               \
 && pip3 install -r requirements_nodoc.txt    \
 && dnf install -y                            \
    $(bindep -b testing | tr '\n' ' ')        \
 && dnf clean all                             \
 && rm -rf /var/cache/yum


WORKDIR /opt/mod/build
ENV BABEL_LIBDIR=/usr/lib64/openbabel3
ENV CXXFLAGS=-Werror
RUN cmake ../ -DBUILD_DOC=no                  \
 -DCMAKE_BUILD_TYPE=Release                   \
 -DCMAKE_MODULE_LINKER_FLAGS="-flto=$j"       \
 -DCMAKE_SHARED_LINKER_FLAGS="-flto=$j"       \
 -DBUILD_TESTING=on                           \
 && make -j $j                                \
 && make tests -j $j                          \
 && make install                              \
 && ctest -j $j --output-on-failure -E cmake_ \
 && rm -rf /opt/mod

WORKDIR /workdir
