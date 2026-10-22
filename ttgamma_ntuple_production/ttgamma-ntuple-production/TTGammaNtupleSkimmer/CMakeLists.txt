# Declare the name of this package:
atlas_subdir( TTGammaNtupleSkimmer None )

atlas_depends_on_subdirs( PUBLIC
                          PathResolver
                          TopDataPreparation
                          EventDuplicateChecker )

# This package uses Boost and ROOT.
find_package( Boost COMPONENTS program_options )
find_package( ROOT REQUIRED COMPONENTS Core Gpad Tree Hist RIO MathCore Graf TreePlayer )

# Build a library that other components can link against.
atlas_add_library( TTGammaNtupleSkimmer Root/*.cxx Root/*.h
                   TTGammaNtupleSkimmer/*.h
                   PUBLIC_HEADERS TTGammaNtupleSkimmer
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES}
                                  ${Boost_LIBRARIES}
                                  PathResolver
                                  TopDataPreparation
                                  EventDuplicateCheckerLib
				  TTGammaEventSaver )

# Install scripts from the package:
atlas_install_scripts( scripts/* )

# Install data files from the package:
atlas_install_data( share/* )

# Install the package's python modules.
atlas_install_python_modules( python/*.py )

# Add the main executable of the skimmer.
atlas_add_executable( ntuple-skimmer
                      util/ntuple-skimmer.cxx
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES}
                                     ${Boost_LIBRARIES}
                                     TTGammaNtupleSkimmer
                                     PathResolver
                                     TopDataPreparation
                                     EventDuplicateCheckerLib
				     TTGammaEventSaver
                    )

