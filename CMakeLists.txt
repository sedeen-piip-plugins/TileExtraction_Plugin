PROJECT( TileExtraction )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

##
## Load the Sedeen dependencies
FIND_PACKAGE( SEDEENSDK REQUIRED 
                HINTS ../../.. 
                "$C/Azadeh/Sedeen Viewer SDK/v5.2.1.384/msvc2012" )


INCLUDE_DIRECTORIES( "${SEDEENSDK_INCLUDE_DIR}" )

LINK_DIRECTORIES( "${SEDEENSDK_LIBRARY_DIR}" )

##
## Build the code into a module library
ADD_LIBRARY( TileExtraction MODULE TileExtraction.cpp TileExtraction.h)

# Link the library against the Sedeen libraries
# NOTE: The QT libraries must be linked first.
TARGET_LINK_LIBRARIES( TileExtraction ${SEDEENSDK_LIBRARIES} )

##
## Install the plugin in the sedeen plugins directory
IF( ${PATHCORE_FOUND} )
  INSTALL( TARGETS TileExtraction 
           LIBRARY DESTINATION "${PATHCORE_DIR}/plugins" )
ENDIF()
