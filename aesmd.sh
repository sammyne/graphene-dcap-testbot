#!/bin/bash

workdir=/opt/intel/sgx-aesm-service/aesm

cd $workdir

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$workdir ./aesm_service --no-daemon
