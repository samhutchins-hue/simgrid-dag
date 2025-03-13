#include "simgrid/s4u.hpp"
#include <xbt/log.h>

// new log category
XBT_LOG_NEW_DEFAULT_CATEGORY(dag_tuto, "lab1 message");

int main(int argc, char *argv[]) {
  // engine instance and load platform
  simgrid::s4u::Engine e(&argc, argv);
  e.load_platform(argv[1]);

  // pointers to hosts
  simgrid::s4u::Host *tremblay = e.host_by_name("Tremblay");
  simgrid::s4u::Host *jupiter = e.host_by_name("Jupiter");

  // Exec and Comm pointers
  // activities
  simgrid::s4u::ExecPtr c1 = simgrid::s4u::Exec::init();
  simgrid::s4u::ExecPtr c2 = simgrid::s4u::Exec::init();
  simgrid::s4u::ExecPtr c3 = simgrid::s4u::Exec::init();
  simgrid::s4u::CommPtr t1 = simgrid::s4u::Comm::sendto_init();

  // name the activities
  c1->set_name("c1");
  c2->set_name("c2");
  c3->set_name("c3");
  t1->set_name("t1");

  // set the amount of work per activity
  c1->set_flops_amount(1e9);
  c2->set_flops_amount(5e9);
  c3->set_flops_amount(2e9);
  t1->set_payload_size(5e8);

  // define the dependencies between the activities
  c1->add_successor(t1);
  t1->add_successor(c3);
  c2->add_successor(c3);

  // set the locatoin of each exec activity and source and destination
  // for the comm activity
  c1->set_host(tremblay);
  c2->set_host(jupiter);
  c3->set_host(jupiter);
  t1->set_source(tremblay);
  t1->set_destination(jupiter);

  // start the execution of activities without dependencies
  c1->start();
  c2->start();

  simgrid::s4u::Exec::on_completion_cb([](simgrid::s4u::Exec const &exec) {
    XBT_INFO("Exec '%s' is complete (start time: %f, finish time: %f)",
             exec.get_cname(), exec.get_start_time(), exec.get_finish_time());
  });

  simgrid::s4u::Comm::on_completion_cb([](simgrid::s4u::Comm const &comm) {
    XBT_INFO("Comm '%s' is complete (start time: %f, finish time: %f)",
             comm.get_cname(), comm.get_start_time(), comm.get_finish_time());
  });

  e.run();
}
