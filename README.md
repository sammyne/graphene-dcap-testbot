# graphene-dcap-testbot

This repository serves to reproduce the error of AESM reporting `Malformed request received (May be forged for attack)`.

Really appreciate if someone could help me solve this error~

## Description
The DCAP quote generation failed for graphene running within docker containers.

The demo app goes in the testbot folder, where
- hello-world.c is the simplified version of [LibOS/shim/test/regression/attestation.c][attestation.c]
- sgx_*.h is copied from Pal/src/host/Linux-SGX folder

The Dockerfile for the base docker image `sammyne/graphene:8bce8e6-ubuntu18.04` sees [here][sammyne/graphene:8bce8e6-ubuntu18.04].

## Steps to reproduce

```bash
docker build -t graphene-dcap-testbot:alpha .
```

## Expected results

No errors.

## Actual results

Logging goes as follows

```bash
root@9f17b8b8dbf6:/graphene# graphene-sgx hello-world
error: Using insecure argv source. Graphene will continue application execution, but this configuration must not be used in production!
opening quote1.dat failed
opening quote2.dat failed
quote.len = 4594
Test quote interface... SUCCESS
aesm_service[24]: Malformed request received (May be forged for attack)
```

## Additional information
- os: Ubuntu 18.04.5 LTS
- docker: 18.09.9, build 039a7df9ba
- sgx driver: 1.36
- sgx: 2.12.100.3
- dcap: dcap1.9.100.3
- graphene: [8bce8e6][graphene]

[attestation.c]: https://github.com/oscarlab/graphene/blob/8bce8e633e2d7f40816cd527060cd539c6f307fa/LibOS/shim/test/regression/attestation.c#L280
[graphene]: https://github.com/oscarlab/graphene/tree/8bce8e633e2d7f40816cd527060cd539c6f307fa
[sammyne/graphene:8bce8e6-ubuntu18.04]: https://github.com/sammyne/ghcr.io/blob/main/graphene/8bce8e6/ubuntu18.04/Dockerfile