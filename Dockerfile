
FROM ubuntu:18.04

RUN apt-get update               && \
    apt-get -y install build-essential git cmake 

RUN apt-get -y install libpq-dev postgresql-server-dev-10

ARG CACHEBUST=1

RUN git clone https://github.com/Tyill/zmey.git

RUN cd zmey && git checkout ver_1_0 

WORKDIR /zmey/build

RUN cmake -B . -S ../core -DCMAKE_BUILD_TYPE=Release && cmake --build . --config Release

CMD [""]


