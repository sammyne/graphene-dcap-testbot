FROM sammyne/graphene:8bce8e6-ubuntu18.04 AS builder

RUN sed -i 's/archive.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list &&\
  sed -i 's/security.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list  &&\
  apt update          &&\
  apt install -y zip

WORKDIR /root/graphene/Examples/hello-world

ADD testbot .

ENV LC_ALL=C.UTF-8 \
    LANG=C.UTF-8

RUN make app && make SGX=1

RUN mkdir /output &&\
  cp hello-world hello-world.manifest hello-world.manifest.sgx hello-world.sig\
    hello-world.token /output

FROM sammyne/sgx-dcap:2.12.100.3-dcap1.9.100.3-ubuntu18.04

RUN sed -i 's/archive.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list &&\
  sed -i 's/security.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list

RUN apt update && apt install -y libprotobuf-c-dev

ENV RA_TLS_ALLOW_OUTDATED_TCB_INSECURE=1

WORKDIR /graphene

COPY --from=builder /usr/local/bin/graphene-sgx /usr/local/bin/graphene-sgx
COPY --from=builder /usr/local/lib/x86_64-linux-gnu/graphene /usr/local/lib/x86_64-linux-gnu/graphene

#COPY --from=builder /usr/local/lib /usr/local/lib
#COPY --from=builder /usr/local/grpc /usr/local/grpc
#COPY --from=builder /usr/local/openssl /usr/local/openssl

COPY --from=builder /output .

CMD graphene-sgx hello-world 
