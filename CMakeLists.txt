# The app_dcca extension
PROJECT("Diameter Credit Control Application" C)

# Compile as a module
FD_ADD_EXTENSION(app_dcca app_dcca.h app_dcca.c)

####
## INSTALL section ##

INSTALL(TARGETS app_dcca
        LIBRARY DESTINATION ${INSTALL_EXTENSIONS_SUFFIX}
        COMPONENT freeDiameter-accounting-server)
