// ======================================================================== //
// Copyright 2009-2017 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "mpiCommon/MPICommon.h"
#include "mpiCommon/async/CommLayer.h"

using namespace ospray;
using namespace mpi;

void setup(int *ac, const char **av)
{
  MPI_Status status;
  mpi::init(ac,av);

  logMPI = true;

  SERIALIZED_MPI_CALL(Barrier(MPI_COMM_WORLD));

  if (world.size <= 1) {
    throw std::runtime_error("No MPI workers found.\n#osp:mpi: Fatal Error "
                             "- OSPRay told to run in MPI mode, but there "
                             "seems to be no MPI peers!?\n#osp:mpi: (Did "
                             "you forget an 'mpirun' in front of your "
                             "application?)");
  }

  if (world.rank == 0) {
    // we're the root
    SERIALIZED_MPI_CALL(Comm_split(mpi::world.comm,1,mpi::world.rank,&app.comm));
    app.makeIntraComm();
    if (logMPI) {
      std::cerr << "#w: app process " << app.rank << '/' << app.size
                << " (global " << world.rank << '/' << world.size << std::endl;
    }

    SERIALIZED_MPI_CALL(Intercomm_create(app.comm, 0, world.comm, 1, 1, &worker.comm));
    if (logMPI) {
      std::cerr << "master: Made 'worker' intercomm (through intercomm_create): "
                << std::hex << std::showbase << worker.comm
                << std::noshowbase << std::dec << std::endl;
    }

    // worker.makeIntracomm();
    worker.makeInterComm();


    // ------------------------------------------------------------------
    // do some simple hand-shake test, just to make sure MPI is
    // working correctly
    // ------------------------------------------------------------------
    serialized(CODE_LOCATION, [&](){
      if (logMPI) {
        std::cerr << "#m: ping-ponging a test message to every worker..."
                  << std::endl;
      }

      for (int i=0;i<worker.size;i++) {
        if (logMPI) {
          std::cerr << "#m: sending tag "<< i << " to worker " << i << std::endl;
        }
        MPI_Send(&i,1,MPI_INT,i,i,worker.comm);
        int reply;
        MPI_Recv(&reply,1,MPI_INT,i,i,worker.comm,&status);
        Assert(reply == i);
      }
      MPI_Barrier(MPI_COMM_WORLD);
    });

    // -------------------------------------------------------
    // at this point, all processes should be set up and synced. in
    // particular:
    // - app has intracommunicator to all workers (and vica versa)
    // - app process(es) are in one intercomm ("app"); workers all in
    //   another ("worker")
    // - all processes (incl app) have barrier'ed, and thus now in sync.
  } else {
    // we're the workers
    SERIALIZED_MPI_CALL(Comm_split(mpi::world.comm,0,mpi::world.rank,&worker.comm));
    worker.makeIntraComm();
    if (logMPI) {
      auto &msg = std::cerr;
      msg << "master: Made 'worker' intercomm (through split): "
          << std::hex << std::showbase << worker.comm
          << std::noshowbase << std::dec << std::endl;

      msg << "#w: app process " << app.rank << '/' << app.size
          << " (global " << world.rank << '/' << world.size << std::endl;
    }

    SERIALIZED_MPI_CALL(Intercomm_create(worker.comm, 0, world.comm, 0, 1, &app.comm));
    app.makeInterComm();

    // ------------------------------------------------------------------
    // do some simple hand-shake test, just to make sure MPI is
    // working correctly
    // ------------------------------------------------------------------
    serialized(CODE_LOCATION, [&](){
      // replying to test-message
      if (logMPI) {
        auto &msg = std::cerr;
        msg << "#w: start-up ping-pong: worker " << worker.rank <<
               " trying to receive tag " << worker.rank << "...\n";
      }
      int reply;
      MPI_Recv(&reply,1,MPI_INT,0,worker.rank,app.comm,&status);
      MPI_Send(&reply,1,MPI_INT,0,worker.rank,app.comm);

      MPI_Barrier(MPI_COMM_WORLD);
    });
  }
}

void shutdown()
{
  async::shutdown();
}

int main(int argc, const char **argv)
{
  setup(&argc, argv);
  shutdown();
  return 0;
}
