###################################################################
## Definitely the most optimized Dockerfile but for now it works ##
###################################################################

FROM ubuntu:20.04 as build

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get install -y bash && \
    apt-get install -y gcc && \
    apt-get install -y g++ && \
    apt-get install -y git && \
    apt-get install -y python3 && \
    apt-get install -y cmake=3.16.3-1ubuntu1.20.04.1 && \
    apt-get install -y wget && \
    apt-get install -y libpcre3 && \
    apt-get install -y libpcre3-dev 
    #&& \
    #mkdir /install

# open62541
WORKDIR /open62541
RUN git clone --depth 1 --branch v1.3.5 https://github.com/open62541/open62541.git && \
    cd open62541 && \
    git submodule update --init --recursive && \
    mkdir build && cd build && \
    #cmake -DCMAKE_INSTALL_PREFIX:PATH=/install -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE -DBUILD_SHARED_LIBS=ON -DUA_BUILD_UNIT_TESTS=OFF -DUA_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUA_NAMESPACE_ZERO=FULL .. && \
    cmake -DBUILD_SHARED_LIBS=ON -DUA_BUILD_UNIT_TESTS=OFF -DUA_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUA_NAMESPACE_ZERO=FULL .. && \
    make && \
    make install

# libyang
WORKDIR /libyang 
RUN wget https://github.com/CESNET/libyang/archive/refs/tags/v1.0.225.tar.gz && \
    tar -vxf v1.0.225.tar.gz && \
    cd libyang-1.0.225 && \
    mkdir build && cd build && \
    #cmake -DCMAKE_INSTALL_PREFIX:PATH=/install -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE .. && \
    cmake .. && \
    make && \
    make install

# sysrepo
WORKDIR /sysrepo 
RUN wget https://github.com/sysrepo/sysrepo/archive/refs/tags/v1.4.122.tar.gz && \
    tar -vxf v1.4.122.tar.gz && \
    cd sysrepo-1.4.122 && \
    mkdir build && cd build && \
    #cmake -DCMAKE_INSTALL_PREFIX:PATH=/install -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE .. && \
    cmake .. && \
    make && \
    make install


FROM ubuntu:20.04 as runtime
RUN apt-get update && \
    apt-get install -y cmake=3.16.3-1ubuntu1.20.04.1 && \
    apt-get install -y g++ && \ 
    apt-get install -y python3 && \
    apt-get install -y libjansson-dev=2.12-1build1 && \
    apt-get install -y libulfius-dev=2.5.2-4 && \
    apt-get install -y uwsc && \
    apt-get install -y sudo && \
    apt-get install -y python3-pip && \
    apt-get install -y supervisor && \
    pip install git+https://github.com/coderanger/supervisor-stdout

#COPY --from=build /install/lib /usr/local/lib
#COPY --from=build /install/bin /usr/local/bin
#COPY --from=build /install/share /usr/local/share
#COPY --from=build /install/include /usr/local/include
COPY --from=build /usr/local/lib /usr/local/lib
COPY --from=build /usr/local/bin /usr/local/bin
COPY --from=build /usr/local/share /usr/local/share
COPY --from=build /usr/local/include /usr/local/include

ENV LD_LIBRARY_PATH=/usr/local/lib
#ENV LD_LIBRARY_PATH=/install/lib
RUN ldconfig

# Build Engineering Framework
COPY . /engineeringframework/

RUN rm -rf /engineeringframework/build

WORKDIR /engineeringframework/build
RUN cmake .. && make

# Install Sysrepo Plugin
RUN mkdir -p /usr/local/lib/sysrepo/plugins
WORKDIR /engineeringframework/src/sysrepo/plugin
RUN sh update_plugin.sh

# Install YANG Modules
WORKDIR /engineeringframework/src/sysrepo
RUN sysrepoctl -i ieee802-dot1q-tsn-types.yang
RUN sysrepoctl -i control-tsn-uni.yang

# Load initial data into sysrepo
RUN sysrepocfg --import=sps_2023.xml -m control-tsn-uni -d startup
RUN sysrepocfg --import=sps_2023.xml -m control-tsn-uni -d running

# Use Supervisord to start the Framework itself and the CNC Prototype
COPY supervisord.conf /etc/supervisor/conf.d/supervisord.conf

WORKDIR /engineeringframework/build
CMD ["supervisord"]