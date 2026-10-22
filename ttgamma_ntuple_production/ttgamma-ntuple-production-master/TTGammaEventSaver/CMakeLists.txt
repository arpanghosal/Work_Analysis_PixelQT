# Declare the name of this package:
atlas_subdir( TTGammaEventSaver None )

# This package depends on other packages:
atlas_depends_on_subdirs( PUBLIC
                          TopEventSelectionTools
                          TopEvent
                          TopConfiguration
                          TopAnalysis
                          AsgTools
                          GammaORTools
                          JetRec
                          PhotonVertexSelection )

# This package uses ROOT:
find_package( ROOT REQUIRED COMPONENTS Core Gpad Tree Hist RIO MathCore Graf )

# Install data files from the package:
atlas_install_scripts( scripts/* )

# Install the package's python modules.
atlas_install_python_modules( python/*.py )

# Generate a CINT dictionary source file:
atlas_add_root_dictionary( TTGammaEventSaver _cintDictSource
                           ROOT_HEADERS Root/LinkDef.h
                           EXTERNAL_PACKAGES ROOT )

# Build a library that other components can link against:
atlas_add_library( TTGammaEventSaver Root/*.cxx Root/*.h
                   TTGammaEventSaver/*.h
                   ${_cintDictSource}
                   PUBLIC_HEADERS TTGammaEventSaver
                   LINK_LIBRARIES TopEventSelectionTools
                                  TopEvent
                                  TopConfiguration
                                  TopAnalysis
                                  AsgTools
                                  GammaORToolsLib
                                  JetRecLib
                                  PhotonVertexSelectionLib
                                  ${ROOT_LIBRARIES}
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} )

# Install data files from the package:
atlas_install_data( DIRECTORY share/* )
