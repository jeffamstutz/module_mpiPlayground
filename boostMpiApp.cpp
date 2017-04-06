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

#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>

#include <iostream>
#include <string>

void tutorial1()
{
  boost::mpi::environment env;
  boost::mpi::communicator world;
  std::cout << "I am process " << world.rank() << " of " << world.size()
            << "." << std::endl;
}

void tutorial_point_communication()
{
  boost::mpi::environment env;
  boost::mpi::communicator world;

  auto rank = world.rank();

  if (rank == 0) {
    world.send(1, 0, std::string("Hello"));
    std::string msg;
    world.recv(1, 1, msg);
    std::cout << msg << "!" << std::endl;
  } else if (rank == 1) {
    std::string msg;
    world.recv(0, 0, msg);
    std::cout << msg << ", ";
    std::cout.flush();
    world.send(0, 1, std::string("world"));
  }
}

void tutorial_non_blocking()
{
  boost::mpi::environment env;
  boost::mpi::communicator world;

  auto rank = world.rank();

  if (rank == 0) {
    boost::mpi::request reqs[2];
    std::string msg, out_msg = "Hello";
    reqs[0] = world.isend(1, 0, out_msg);
    reqs[1] = world.irecv(1, 1, msg);
    boost::mpi::wait_all(reqs, reqs + 2);
    std::cout << msg << "!" << std::endl;
  } else if (rank == 1) {
    boost::mpi::request reqs[2];
    std::string msg, out_msg = "world";
    reqs[0] = world.isend(0, 1, out_msg);
    reqs[1] = world.irecv(0, 0, msg);
    boost::mpi::wait_all(reqs, reqs + 2);
    std::cout << msg << ", ";
  }
}

int main(int argc, char **argv)
{
  //tutorial_hello();
  //tutorial_point_communication();
  tutorial_non_blocking();

  return 0;
}
