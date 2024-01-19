FROM archlinux

ARG j=7
ENV VIRTUAL_ENV=/opt/venv

RUN pacman -Suy --noconfirm python-virtualenv
RUN python -m venv --system-site-packages $VIRTUAL_ENV 
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

WORKDIR /opt/mod
COPY ./build/mod-*.tar.gz ./
RUN tar xzf mod-*.tar.gz --strip-components=1
RUN pip install -r requirements_nodoc.txt                   \
 && pacman -Suy --noconfirm                                 \
    $(bindep -b testing | tr '\n' ' ')                      \
 && rm -rf /var/cache/pacman

WORKDIR /opt/mod/build
ENV CXXFLAGS=-Werror -Wno-error=maybe-uninitialized
RUN cmake ../ -DBUILD_DOC=no \
 -DCMAKE_BUILD_TYPE=Release                                                    \
 -DCMAKE_MODULE_LINKER_FLAGS="-flto=$j" -DCMAKE_SHARED_LINKER_FLAGS="-flto=$j" \
 -DBUILD_TESTING=on                           \
 && make -j $j                                \
 && make tests -j $j                          \
 && make install                              \
 && ctest -j $j --output-on-failure -E cmake_ \
 && rm -rf /opt/mod

WORKDIR /workdir
