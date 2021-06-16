# graphene-dcap-testbot

This repository serves to reproduce the error of AESM reporting `Malformed request received (May be forged for attack)`.

Really appreciate if someone could help me solve this error~

## Description
The DCAP quote generation failed for graphene running within docker containers, but it's fine when running the official DCAP [SampleCodes][dcap-samplecode] by Intel.

The specific error is reported by the AESM, and goes as 
```bash
Malformed request received (May be forged for attack)
```

The demo app goes in the testbot folder, where
- hello-world.c is the simplified version of [LibOS/shim/test/regression/attestation.c][attestation.c]
- sgx_*.h is copied from Pal/src/host/Linux-SGX folder

The Dockerfile for the base docker image `sammyne/graphene:8bce8e6-ubuntu18.04` sees [here][sammyne/graphene:8bce8e6-ubuntu18.04].

## Steps to reproduce

1. Build and start PCCS
    ```bash
    cd intel-pccs-1.9
    docker build -t intel-pccs:1.9 .

    docker run --rm -d \
      --name intel-pccs \
      --network host \
      -v $PWD/config.json:/pccs/config/default.json \
    ```
2. Update the IP within qcnl.conf to point to the actual address of PCCS service.
  - e.g. if your machine bears IP as `5.6.7.8`, you should replace the `1.2.3.4` with `5.6.7.8`

3. Build and run the demo app
    ```bash
    docker build -t graphene-shim-dcap:alpha .

    docker run --rm -it \
      --name graphene-shim-dcap \
      -v $PWD/qcnl.conf:/etc/sgx_default_qcnl.conf     \
      -v $PWD/aesmd.sh:/graphene/aesmd.sh     \
      --device /dev/kmsg:/dev/kmsg    \
      --device /dev/gsgx:/dev/gsgx    \
      --device /dev/sgx:/dev/sgx      \
      graphene-shim-dcap:alpha bash

    # within docker container, start aesmd
    bash aesmd.sh

    # open another terminal, and attach into container 'graphene-shim-dcap'
    docker exec -it graphene-shim-dcap bash

    # and run 
    graphene-sgx hello-world
    ```

## Expected results

No errors.

## Actual results

Logging goes as follows

```bash
root@9f17b8b8dbf6:/graphene# graphene-sgx hello-world
error: Using insecure argv source. Graphene will continue application execution, but this configuration must not be used in production!
Test quote interface... SUCCESS
aesm_service[24]: Malformed request received (May be forged for attack)
```

## Additional information
- os: Ubuntu 18.04.5 LTS
- docker: 18.09.9, build 039a7df9ba
- sgx driver: 1.36
- sgx: 2.12.100.3
- dcap: dcap1.9.100.3
- PCCS: DCAP_1.9 as built according to [intel-pccs-1.9/Dockerfile](intel-pccs-1.9/Dockerfile)
- graphene: [8bce8e6][graphene]
- graphene's protobuf source code for AESM goes in the aesm-pb folder

[attestation.c]: https://github.com/oscarlab/graphene/blob/8bce8e633e2d7f40816cd527060cd539c6f307fa/LibOS/shim/test/regression/attestation.c#L280
[dcap-samplecode]: https://github.com/intel/SGXDataCenterAttestationPrimitives/tree/DCAP_1.9/SampleCode
[graphene]: https://github.com/oscarlab/graphene/tree/8bce8e633e2d7f40816cd527060cd539c6f307fa
[sammyne/graphene:8bce8e6-ubuntu18.04]: https://github.com/sammyne/ghcr.io/blob/main/graphene/8bce8e6/ubuntu18.04/Dockerfile
