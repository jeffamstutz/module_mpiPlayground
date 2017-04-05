## ======================================================================== ##
## Copyright 2009-2017 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

option(OSPRAY_MODULE_MPIPLAYGROUND "Build 'MPI Playground' Module" ON)

if (OSPRAY_MODULE_MPIPLAYGROUND)

  ospray_build_component(mpiCommon)

  # if mpi mode is enabled, we have to configure the right mpi
  # compiler etc.
  ospray_configure_mpi()

  ospray_create_application(ospMpiPlayground
    testApp.cpp
  LINK
    ospray_common
    ospray_mpi_common
    ospray_module_mpi
  )

endif()
