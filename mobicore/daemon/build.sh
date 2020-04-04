#!/bin/bash

source ../../../../setup.sh
if [ $? -ne 0 ]; then
	echo "Error: Could not source setup.sh!"
	exit 1
fi

ROOT_PATH=$(dirname $(readlink -f $BASH_SOURCE))

### 
###   Android NDK path
### 
PLATFORM=Generic
MODE=debug

echo "Building <t-base Daemon..."
   
# run NDK build
${NDK_BUILD} \
	-B \
	NDK_DEBUG=1 \
	NDK_PROJECT_PATH=$ROOT_PATH \
	NDK_APPLICATION_MK=$ROOT_PATH/Application.mk \
	NDK_MODULE_PATH=. \
	NDK_APP_OUT=$ROOT_PATH/Out/_build \
	APP_BUILD_SCRIPT=$ROOT_PATH/Android.mk \
	APP_OPTIM=$MODE