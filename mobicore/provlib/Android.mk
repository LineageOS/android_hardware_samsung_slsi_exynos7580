# =============================================================================
#
# Makefile pointing to all makefiles within the project.
#
# =============================================================================

PROJECT_PATH := $(call my-dir)

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))

# Include the Scripts
include $(PROJECT_PATH)/jni/Android.mk
