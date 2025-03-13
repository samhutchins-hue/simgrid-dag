#include "simgrid/s4u.hpp"
#include <string>
#include <vector>
#include <xbt/log.h>

// Define a default logging category for the DAG pipeline
XBT_LOG_NEW_DEFAULT_CATEGORY(pipeline_dag, "Pipeline DAG simulation");

int main(int argc, char *argv[]) {
  // Initialize the engine and load the platform
  simgrid::s4u::Engine e(&argc, argv);
  e.load_platform(argv[1]);

  // Retrieve hosts from the platform
  simgrid::s4u::Host *host1 = e.host_by_name("Host1");
  simgrid::s4u::Host *host2 = e.host_by_name("Host2"); // Should have 2 cores
  simgrid::s4u::Host *host3 = e.host_by_name("Host3");

  int num_tasks = 4;
  // We'll store pointers to Stage1 activities so we can start them later
  std::vector<simgrid::s4u::ExecPtr> stage1_tasks;

  // Build the pipeline chains without starting any activity
  for (int i = 0; i < num_tasks; ++i) {
    // Create Stage 1 activity
    simgrid::s4u::ExecPtr stage1 = simgrid::s4u::Exec::init();
    stage1->set_name(("Stage1_" + std::to_string(i)).c_str());
    stage1->set_flops_amount(1e9);
    stage1->set_host(host1);

    // Create Comm1 (communication from Stage1 to Stage2)
    simgrid::s4u::CommPtr comm1 = simgrid::s4u::Comm::sendto_init();
    comm1->set_name(("Comm1_" + std::to_string(i)).c_str());
    comm1->set_payload_size(1e8);
    comm1->set_source(host1);
    comm1->set_destination(host2);

    // Create Stage 2 activity
    simgrid::s4u::ExecPtr stage2 = simgrid::s4u::Exec::init();
    stage2->set_name(("Stage2_" + std::to_string(i)).c_str());
    stage2->set_flops_amount(2e9);
    stage2->set_host(host2);

    // Create Comm2 (communication from Stage2 to Stage3)
    simgrid::s4u::CommPtr comm2 = simgrid::s4u::Comm::sendto_init();
    comm2->set_name(("Comm2_" + std::to_string(i)).c_str());
    comm2->set_payload_size(1e8);
    comm2->set_source(host2);
    comm2->set_destination(host3);

    // Create Stage 3 activity
    simgrid::s4u::ExecPtr stage3 = simgrid::s4u::Exec::init();
    stage3->set_name(("Stage3_" + std::to_string(i)).c_str());
    stage3->set_flops_amount(1.5e9);
    stage3->set_host(host3);

    // Build the dependency chain
    stage1->add_successor(comm1);
    comm1->add_successor(stage2);
    stage2->add_successor(comm2);
    comm2->add_successor(stage3);

    // Detach the communication activities now.
    // Because none of the pipeline tasks have started yet, detach() is allowed.
    comm1->detach();
    comm2->detach();

    // Save the Stage1 pointer for later starting
    stage1_tasks.push_back(stage1);
  }

  // Now that all pipelines are fully built and detach() has been called,
  // start all Stage1 activities (which will trigger the entire chain).
  for (auto &s : stage1_tasks) {
    s->start();
  }

  // Set up logging callbacks to monitor activity completions.
  simgrid::s4u::Exec::on_completion_cb([](simgrid::s4u::Exec const &exec) {
    XBT_INFO("Exec '%s' complete (start: %f, finish: %f)", exec.get_cname(),
             exec.get_start_time(), exec.get_finish_time());
  });
  simgrid::s4u::Comm::on_completion_cb([](simgrid::s4u::Comm const &comm) {
    XBT_INFO("Comm '%s' complete (start: %f, finish: %f)", comm.get_cname(),
             comm.get_start_time(), comm.get_finish_time());
  });

  // Run the simulation
  e.run();
  return 0;
}
