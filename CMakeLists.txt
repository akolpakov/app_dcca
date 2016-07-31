# The app_dcca extension
PROJECT("Diameter Credit Control Application" C)

# Find LibCURL
FIND_PACKAGE(CURL REQUIRED)
INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})

# Compile as a module
FD_ADD_EXTENSION(app_dcca app_dcca.h app_dcca.c parson.h parson.c rest_api.h rest_api.c)
TARGET_LINK_LIBRARIES(app_dcca ${CURL_LIBRARIES})

####
## INSTALL section ##

INSTALL(TARGETS app_dcca
        LIBRARY DESTINATION ${INSTALL_EXTENSIONS_SUFFIX}
        COMPONENT freeDiameter-credit-control-application)
