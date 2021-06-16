# see https://software.intel.com/content/www/us/en/develop/articles/intel-software-guard-extensions-data-center-attestation-primitives-quick-install-guide.html
FROM alpine:3.13.5 as builder1

RUN apk add openssl git

WORKDIR /output/pki

RUN openssl genrsa -out private.pem 2048                                      &&\
  openssl req -new -key private.pem -out csr.pem -subj '/C=TX'                &&\
  openssl x509 -req -days 365 -in csr.pem -signkey private.pem  -out file.crt

WORKDIR /output/code

RUN git clone -b DCAP_1.9                                         \
    https://github.com/intel/SGXDataCenterAttestationPrimitives .

FROM node:14.17.0-buster-slim AS builder2

ENV DEBIAN_FRONTEND=noninteractive

RUN sed -i 's/deb.debian.org/mirrors.ustc.edu.cn/g' /etc/apt/sources.list

RUN apt update && apt install -y zip sqlite3 python build-essential

FROM builder2 AS pckLib

WORKDIR /pck

COPY --from=builder1 /output/code .

RUN cd tools/PCKCertSelection           &&\
  make                                  &&\
  mkdir /output                         &&\
  cp out/libPCKCertSelection.so /output

FROM builder2 AS pccs

WORKDIR /output

COPY --from=builder1 /output/code/QuoteGeneration/pccs .

RUN npm config set registry https://registry.npm.taobao.org --global  &&\
  npm config set disturl https://npm.taobao.org/dist --global         &&\
  npm install &&\
  npm rebuild

FROM builder2

WORKDIR /pccs

COPY --from=builder1 /output/pki ssl_key
COPY --from=pckLib /output lib
COPY --from=pccs /output .

CMD NODE_ENV=dev && node -r esm pccs_server.js
