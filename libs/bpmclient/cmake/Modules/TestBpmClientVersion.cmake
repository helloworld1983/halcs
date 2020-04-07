# Read header file and extract version numbers

file(READ
    "${PROJECT_SOURCE_DIR}/include/bpm_client_classes.h"
    _BPMCLIENT_H_CONTENTS
)

string(REGEX REPLACE
    ".*#define BPM_CLIENT_VERSION_MAJOR ([0-9]+).*"
    "\\1" BPMCLIENT_VERSION_MAJOR
    "${_BPMCLIENT_H_CONTENTS}"
)
string(REGEX REPLACE
    ".*#define BPM_CLIENT_VERSION_MINOR ([0-9]+).*"
    "\\1" BPMCLIENT_VERSION_MINOR
    "${_BPMCLIENT_H_CONTENTS}"
)
string(REGEX REPLACE
    ".*#define BPM_CLIENT_VERSION_PATCH ([0-9]+).*"
    "\\1" BPMCLIENT_VERSION_PATCH
    "${_BPMCLIENT_H_CONTENTS}"
)

set(BPMCLIENT_VERSION
    "${BPMCLIENT_VERSION_MAJOR}.${BPMCLIENT_VERSION_MINOR}.${BPMCLIENT_VERSION_PATCH}"
)

message(STATUS "Detected BPMCLIENT Version - ${BPMCLIENT_VERSION}")
