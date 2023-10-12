###################################################################
## Definitely the most optimized Dockerfile but for now it works ##
###################################################################

FROM ubuntu:20.04 as build

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update
RUN apt-get install -y bash
RUN apt-get install -y gcc
RUN apt-get install -y g++
RUN apt-get install -y git
RUN apt-get install -y python3 
RUN apt-get install -y cmake=3.16.3-1ubuntu1.20.04.1
RUN apt-get install -y wget
RUN apt-get install -y libpcre3
RUN apt-get install -y libpcre3-dev

RUN mkdir /install

# open62541
WORKDIR /open62541
RUN git clone --depth 1 --branch v1.3.5 https://github.com/open62541/open62541.git && \
    cd open62541 && \
    git submodule update --init --recursive && \
    mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/install -DBUILD_SHARED_LIBS=ON -DUA_BUILD_UNIT_TESTS=OFF -DUA_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUA_NAMESPACE_ZERO=FULL .. && \
    make && \
    make install

# libyang
WORKDIR /libyang 
RUN wget https://github.com/CESNET/libyang/archive/refs/tags/v1.0.225.tar.gz && \
    tar -vxf v1.0.225.tar.gz && \
    cd libyang-1.0.225 && \
    mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/install .. && \
    make && \
    make install

# sysrepo
WORKDIR /sysrepo 
RUN wget https://github.com/sysrepo/sysrepo/archive/refs/tags/v1.4.122.tar.gz && \
    tar -vxf v1.4.122.tar.gz && \
    cd sysrepo-1.4.122 && \
    mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/install .. && \
    make && \
    make install


FROM ubuntu:20.04 as runtime
RUN apt-get update
RUN apt-get install -y cmake=3.16.3-1ubuntu1.20.04.1
RUN apt-get install -y g++
RUN apt-get install -y python3 
RUN apt-get install -y libjansson-dev=2.12-1build1
RUN apt-get install -y libulfius-dev=2.5.2-4
RUN apt-get install -y uwsc
RUN apt-get install -y sudo

COPY --from=build /install/lib /usr/lib
COPY --from=build /install/bin /usr/bin
COPY --from=build /install/share /usr/share
COPY --from=build /install/include /usr/include

# Build Engineering Framework
COPY . /engineeringframework/
RUN rm -rf /engineeringframework/build
WORKDIR /engineeringframework/build
RUN cmake ..
RUN make

# Install Sysrepo Plugin
RUN mkdir -p /usr/local/lib/sysrepo/plugins
WORKDIR /engineeringframework/src/sysrepo/plugin
RUN sh update_plugin.sh

ENTRYPOINT [ "/engineeringframework/build/MainModule" ]