#include "simgrid/s4u.hpp"
#include <xbt/log.h>

// Define a default logging category
XBT_LOG_NEW_DEFAULT_CATEGORY(pipeline_dag, "Pipeline DAG simulation");

int main(int argc, char *argv[]) {
  // Initialize engine and load platform from XML file
  simgrid::s4u::Engine e(&argc, argv);
  e.load_platform(argv[1]);

  // Retrieve hosts from the platform
  simgrid::s4u::Host *host1 = e.host_by_name("Host1");
  simgrid::s4u::Host *host2 = e.host_by_name("Host2");
  simgrid::s4u::Host *host3 = e.host_by_name("Host3");

  // Stage 1: Initial processing ---
  simgrid::s4u::ExecPtr stage1 = simgrid::s4u::Exec::init();
  stage1->set_name("Stage1");
  stage1->set_flops_amount(1e9); // Example computation cost
  stage1->set_host(host1);

  // Communication from Stage 1 to Stage 2
  simgrid::s4u::CommPtr comm1 = simgrid::s4u::Comm::sendto_init();
  comm1->set_name("Comm1");
  comm1->set_payload_size(1e8); // Example payload size
  comm1->set_source(host1);
  comm1->set_destination(host2);

  // Stage 2: Intermediate processing
  simgrid::s4u::ExecPtr stage2 = simgrid::s4u::Exec::init();
  stage2->set_name("Stage2");
  stage2->set_flops_amount(2e9);
  stage2->set_host(host2);

  // Communication from Stage 2 to Stage 3
  simgrid::s4u::CommPtr comm2 = simgrid::s4u::Comm::sendto_init();
  comm2->set_name("Comm2");
  comm2->set_payload_size(1e8);
  comm2->set_source(host2);
  comm2->set_destination(host3);

  // Stage 3: Final processing
  simgrid::s4u::ExecPtr stage3 = simgrid::s4u::Exec::init();
  stage3->set_name("Stage3");
  stage3->set_flops_amount(1.5e9);
  stage3->set_host(host3);

  // Define dependencies to form the pipeline:
  // Stage1 -> Comm1 -> Stage2 -> Comm2 -> Stage3

  // Stage1 completes and triggers the communication to Stage2.
  stage1->add_successor(comm1);
  // Once the communication completes, Stage2 starts.
  comm1->add_successor(stage2);

  // Similarly, Stage2 triggers the communication to Stage3.
  stage2->add_successor(comm2);
  comm2->add_successor(stage3);

  // Set up callbacks for logging activity completions.
  simgrid::s4u::Exec::on_completion_cb([](simgrid::s4u::Exec const &exec) {
    XBT_INFO("Exec '%s' is complete (start: %f, finish: %f)", exec.get_cname(),
             exec.get_start_time(), exec.get_finish_time());
  });
  simgrid::s4u::Comm::on_completion_cb([](simgrid::s4u::Comm const &comm) {
    XBT_INFO("Comm '%s' is complete (start: %f, finish: %f)", comm.get_cname(),
             comm.get_start_time(), comm.get_finish_time());
  });

  // Start the pipeline by launching stage1.
  stage1->start();

  // Run the simulation.
  e.run();
  return 0;
}
